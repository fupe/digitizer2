#include "calibrationengine.h"
#include <QDebug>
#include <algorithm>
#include <numeric>
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <QPair>

static QPointF forwardPoint(const Angles& angle, double arm1, double arm2)
{
    double alphaRad = angle.alfa * M_PI / 180.0;
    double betaRad  = angle.beta * M_PI / 180.0;
    double x1 = std::cos(alphaRad) * arm1;
    double y1 = std::sin(alphaRad) * arm1;
    double x2 = x1 + std::cos(alphaRad - betaRad - M_PI) * arm2;
    double y2 = y1 + std::sin(alphaRad - betaRad - M_PI) * arm2;
    return QPointF(x2, y2);
}



CalibrationEngine::CalibrationEngine(double arm1, double arm2)
 : arm1_(arm1), arm2_(arm2)
{

qDebug() << "ted pocitam s ramenama  "  << arm1  << "  "  << arm2 ;

}

double CalibrationEngine::getArm1()
{
    return arm1_;
}

double CalibrationEngine::getArm2()
{
    return arm2_;
}

void CalibrationEngine::setAngles(const QVector<Angles>& angles) {
    qDebug()<<"--Set angles--";
    this->angles_ = angles;
}

const QVector<Angles>& CalibrationEngine::getAngles() const {
    qDebug()<<"--Get angles--";
    return angles_;
}


void CalibrationEngine::computeOpositPoints() {
    qDebug() << "--computeOpositPoints--";
    double delta = arm1_ * 0.01;

    betaMinIndex_ = -1;
    betaMinOpositIndex_ = -1;
    alfaMinIndex_ = -1;
    alfaMinOpositIndex_ = -1;
    double minDiffArm1 = std::numeric_limits<double>::max();
    double minDiffArm2 = std::numeric_limits<double>::max();

    result_.clear();
    QVector<double> endAngles;

    for (int i = 0; i < angles_.size(); ++i) {
        const Angles& angle = angles_[i];
        QPointF pos = forwardPoint(angle, arm1_, arm2_);

        CalibrationPoint pt;
        pt.position = pos;
        pt.index = i;

        double alphaRad = angle.alfa * M_PI / 180.0;
        double betaRad  = angle.beta * M_PI / 180.0;

        double x1_up1 = std::cos(alphaRad) * (arm1_ + delta);
        double y1_up1 = std::sin(alphaRad) * (arm1_ + delta);
        double x2_up1 = x1_up1 + std::cos(alphaRad - betaRad - M_PI) * arm2_;
        double y2_up1 = y1_up1 + std::sin(alphaRad - betaRad - M_PI) * arm2_;
        pt.diffArm1 = QLineF(pos, QPointF(x2_up1, y2_up1)).length();

        double x2_up2 = std::cos(alphaRad) * arm1_ + std::cos(alphaRad - betaRad - M_PI) * (arm2_ + delta);
        double y2_up2 = std::sin(alphaRad) * arm1_ + std::sin(alphaRad - betaRad - M_PI) * (arm2_ + delta);
        pt.diffArm2 = QLineF(pos, QPointF(x2_up2, y2_up2)).length();

        if (pt.diffArm1 < minDiffArm1) { minDiffArm1 = pt.diffArm1; betaMinIndex_ = i; }
        if (pt.diffArm2 < minDiffArm2) { minDiffArm2 = pt.diffArm2; alfaMinIndex_ = i; }

        result_.append(pt);
        endAngles.append(std::atan2(pos.y(), pos.x()));
    }

    // robust circle fit
    QVector<QPointF> positions;
    for (const CalibrationPoint& pt : result_) positions.append(pt.position);

    auto fitCircle = [](const QVector<QPointF>& pts, QPointF& center, double& radius) {
        int N = pts.size();
        if (N < 3) { center = QPointF(); radius = 0; return; }
        Eigen::MatrixXd A(N,3);
        Eigen::VectorXd b(N);
        for (int i=0;i<N;++i) {
            double x = pts[i].x();
            double y = pts[i].y();
            A(i,0)=2*x; A(i,1)=2*y; A(i,2)=1.0;
            b(i)=x*x + y*y;
        }
        Eigen::Vector3d x = A.colPivHouseholderQr().solve(b);
        center = QPointF(x[0], x[1]);
        radius = std::sqrt(x[2] + x[0]*x[0] + x[1]*x[1]);
    };

    fitCircle(positions, circleCenter_, circleRadius_);
    QVector<double> residuals;
    double totalErr = 0.0;
    for (const QPointF& p : positions) {
        double dist = QLineF(p, circleCenter_).length();
        double err = std::abs(dist - circleRadius_);
        residuals.append(err);
        totalErr += err;
    }
    double avgErr = residuals.isEmpty()?0.0:totalErr/residuals.size();
    double threshold = avgErr * 3.0;
    QVector<QPointF> filtered;
    for (int i=0;i<positions.size();++i)
        if (residuals[i] <= threshold) filtered.append(positions[i]);
    if (filtered.size() >=3 && filtered.size()<positions.size())
        fitCircle(filtered, circleCenter_, circleRadius_);

    totalErr = 0.0;
    maxErrorCircleIndex_ = -1;
    maxErrorCircle_ = 0.0;
    for (int i=0;i<result_.size();++i) {
        double dist = QLineF(result_[i].position, circleCenter_).length();
        double err = std::abs(dist - circleRadius_);
        totalErr += err;
        if (err > maxErrorCircle_) { maxErrorCircle_ = err; maxErrorCircleIndex_ = i; }
    }
    averageCircleFitError_ = result_.isEmpty()?0.0:totalErr/result_.size();

    // search opposite points
    std::vector<std::pair<double,int>> sorted;
    sorted.reserve(endAngles.size());
    for (int i=0;i<endAngles.size();++i) sorted.push_back({endAngles[i], i});
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b){return a.first<b.first;});
    auto comp = [](const std::pair<double,int>& a, const std::pair<double,int>& b){return a.first<b.first;};

    for (int i=0; i<result_.size(); ++i) {
        double target = endAngles[i] + M_PI;
        if (target > M_PI) target -= 2*M_PI;
        if (target < -M_PI) target += 2*M_PI;
        auto it = std::lower_bound(sorted.begin(), sorted.end(), std::make_pair(target,-1), comp);
        int idx1 = (it==sorted.end())? sorted.front().second : it->second;
        int idx0 = (it==sorted.begin())? sorted.back().second : std::prev(it)->second;
        double d1 = QLineF(result_[i].position, result_[idx1].position).length();
        double d0 = QLineF(result_[i].position, result_[idx0].position).length();
        int farIndex; double maxDist;
        if (d1 > d0){ farIndex=idx1; maxDist=d1;} else {farIndex=idx0; maxDist=d0;}

        result_[i].opposite = result_[farIndex].position;
        result_[i].oppositeIndex = farIndex;
        result_[i].distanceToOpposite = maxDist;
        result_[i].diff = maxDist;
        if (i == betaMinIndex_) betaMinOpositIndex_ = farIndex;
        if (i == alfaMinIndex_) alfaMinOpositIndex_ = farIndex;
    }

    double sum = 0.0;
    for (const auto& pt : result_) sum += pt.distanceToOpposite;
    average_dist_ = result_.isEmpty()?0.0:sum/result_.size();

    double maxError = 0.0;
    maxErrorIndex_ = -1;
    for (int i=0;i<result_.size();++i) {
        double error = std::abs(result_[i].distanceToOpposite - average_dist_);
        if (error > maxError) { maxError = error; maxErrorIndex_ = i; }
    }
}



CalibrationResult CalibrationEngine::optimizeArmsGrid(double referenceDistance,
                                                       double stepArm1,
                                                      double stepArm2,
                                                      int steps)
{
    qDebug()<<"--optimizeArmsGrid--";
    double bestArm1 = arm1_;
    double bestArm2 = arm2_;
    double minDeviation = std::numeric_limits<double>::max();

    double baseArm1 = arm1_ - (steps/2) * stepArm1;
    double baseArm2 = arm2_ - (steps/2) * stepArm2;
    double sum;

    for (int i = 0; i <= steps; ++i)
    {
        double testArm1 = baseArm1 + i * stepArm1;
        for (int j = 0; j <= steps; ++j)
        {
            double testArm2 = baseArm2 + j * stepArm2;

            CalibrationEngine testEngine(testArm1, testArm2);
            testEngine.setAngles(angles_);
            testEngine.computeOpositPoints();
/*
            DeviationResult dev = computeMaxDeviation(testEngine.getAngles(), testEngine.points(), testArm1, testArm2, percent);
            qDebug() << " minIndexArm1 " << dev.minIndexArm1 << " mixDeviationArm1" << dev.minDeviationArm1 << " maxIndexArm1 " << dev.maxIndexArm1 << " maxDeviationArm1 " << dev.maxDeviationArm1;
            qDebug() << " minIndexArm2 " << dev.minIndexArm2 << " mixDeviationArm2" << dev.minDeviationArm2 << " maxIndexArm2 " << dev.maxIndexArm2 << " maxDeviationArm2 " << dev.maxDeviationArm2;

            qDebug()<< " devresult.sum arm1 " <<dev.sumDeviationArm1;
            qDebug()<< " devresult.sum arm2 " <<dev.sumDeviationArm2;
            double averageDeviation = dev.averageDeviation();  // můžeš i změnit na maxDeviation nebo jinou metriku
            qDebug()<< " devresult.sum average " << averageDeviation << "arm1 " << testArm1 << "arm2 " <<testArm2;
            if (std::abs(averageDeviation ) < minDeviation)
            {
                qDebug()<< " new value min deviation " <<  minDeviation;
                minDeviation = std::abs(averageDeviation);
                bestArm1 = testArm1;
                bestArm2 = testArm2;
            }

            qDebug() << "Grid test: arm1=" << testArm1 << " arm2=" << testArm2
                     << " deviation=" << averageDeviation << " diff=" << std::abs(averageDeviation);*/
            sum = testEngine.totalDeviationFromReference(referenceDistance);
            qDebug() << "Součet všech odchylek grid:" << sum << " pro ramena " << testArm1 << "  " << testArm2;
            if (sum < minDeviation)
            {
               minDeviation = sum;
               bestArm1=testArm1;
               bestArm2=testArm2;
            }
        }
        qDebug() << "nejlepsi odchylka:" << minDeviation << " arm1 " << bestArm1 << " arm2  " << bestArm2  ;
    }

    return CalibrationResult{bestArm1, bestArm2};
}



CalibrationResult CalibrationEngine::optimizeArms(double referenceDistance,
                                                             double stepArm1,
                                                             double stepArm2,
                                                             int steps,
                                                             int betaMinIndex,
                                                             int betaMinOpositIndex,
                                                             int alfaMinIndex,
                                                             int alfaMinOpositIndex)
{
    qDebug()<<"--optimizeArms--";
    double bestArm1 = arm1_;
    double bestArm2 = arm2_;
    double minTotalDeviation = std::numeric_limits<double>::max();
    double minDeviation = std::numeric_limits<double>::max();

    double baseArm1 = arm1_ - (steps/2) * stepArm1;
    double baseArm2 = arm2_ - (steps/2) * stepArm2;
    double totalDeviation = 0;
    double sum = 0;

    for (int i = 0; i <= steps; ++i)

    {

        double testArm1 = baseArm1 + i * stepArm1;

        for (int j = 0; j <= steps; ++j)
        {

            double testArm2 = baseArm2 + j * stepArm2;

            // ---------- Výpočet vzdálenosti pro rameno 1 ----------
            const Angles& betaA = angles_.at(betaMinIndex);
            const Angles& betaB = angles_.at(betaMinOpositIndex);

            double x1a = cos(betaA.alfa * M_PI / 180.0) * testArm1;
            double y1a = sin(betaA.alfa * M_PI / 180.0) * testArm1;
            double x2a = x1a + cos((betaA.alfa - betaA.beta - 180.0) * M_PI / 180.0) * testArm2;
            double y2a = y1a + sin((betaA.alfa - betaA.beta - 180.0) * M_PI / 180.0) * testArm2;

            double x1b = cos(betaB.alfa * M_PI / 180.0) * testArm1;
            double y1b = sin(betaB.alfa * M_PI / 180.0) * testArm1;
            double x2b = x1b + cos((betaB.alfa - betaB.beta - 180.0) * M_PI / 180.0) * testArm2;
            double y2b = y1b + sin((betaB.alfa - betaB.beta - 180.0) * M_PI / 180.0) * testArm2;

            double dist1 = QLineF(QPointF(x2a, y2a), QPointF(x2b, y2b)).length();


            // ---------- Výpočet vzdálenosti pro rameno 2 ----------
            const Angles& alfaA = angles_.at(alfaMinIndex);

            const Angles& alfaB = angles_.at(alfaMinOpositIndex);
            double x1c = cos(alfaA.alfa * M_PI / 180.0) * testArm1;
            double y1c = sin(alfaA.alfa * M_PI / 180.0) * testArm1;
            double x2c = x1c + cos((alfaA.alfa - alfaA.beta - 180.0) * M_PI / 180.0) * testArm2;
            double y2c = y1c + sin((alfaA.alfa - alfaA.beta - 180.0) * M_PI / 180.0) * testArm2;
            double x1d = cos(alfaB.alfa * M_PI / 180.0) * testArm1;
            double y1d = sin(alfaB.alfa * M_PI / 180.0) * testArm1;
            double x2d = x1d + cos((alfaB.alfa - alfaB.beta - 180.0) * M_PI / 180.0) * testArm2;
            double y2d = y1d + sin((alfaB.alfa - alfaB.beta - 180.0) * M_PI / 180.0) * testArm2;
            double dist2 = QLineF(QPointF(x2c, y2c), QPointF(x2d, y2d)).length();

            // ---------- Porovnání s cílovou vzdáleností ----------
            double deviation1 = std::abs(dist1 - referenceDistance);
            double deviation2 = std::abs(dist2 - referenceDistance);
            totalDeviation = deviation1 + deviation2;
            if (totalDeviation < minTotalDeviation) {
                minTotalDeviation = totalDeviation;
                bestArm1 = testArm1;
                bestArm2 = testArm2;
            }

            qDebug() << " difer - kazde ramen Arm1:" << testArm1 << "Arm2:" << testArm2
                     << "Dev1:" << deviation1 << "Dev2:" << deviation2;

            //----------------
            sum = this->totalDeviationFromReference(referenceDistance);
            qDebug() << "Součet všech odchylek deriv :" << sum << " pro ramena " << testArm1 << "  " << testArm2;
            if (sum < minDeviation)
            {
               minDeviation = sum;
               bestArm1=testArm1;
               bestArm2=testArm2;
               qDebug() << " total  - kazde ramen Arm1:" << testArm1 << "Arm2:" << testArm2 ;
            }

            //---------------
        }
    }


    return CalibrationResult{bestArm1, bestArm2};
}




struct ArmFunctor : Eigen::DenseFunctor<double> {
    const QVector<Angles>& angles;
    const QVector<QPair<int,int>>& pairs;
    double reference;
    ArmFunctor(const QVector<Angles>& a, const QVector<QPair<int,int>>& p, double ref)
        : Eigen::DenseFunctor<double>(2, p.size()), angles(a), pairs(p), reference(ref) {}
    int operator()(const Eigen::VectorXd &x, Eigen::VectorXd &fvec) const {
        double a1 = x[0], a2 = x[1];
        for (int k=0;k<pairs.size();++k) {
            const Angles& ai = angles[pairs[k].first];
            const Angles& aj = angles[pairs[k].second];
            QPointF p1 = forwardPoint(ai, a1, a2);
            QPointF p2 = forwardPoint(aj, a1, a2);
            double dist = QLineF(p1, p2).length();
            fvec[k] = dist - reference;
        }
        return 0;
    }
};

CalibrationResult CalibrationEngine::optimizeArmsLM(double referenceDistance)
{
    QVector<QPair<int,int>> pairs;
    for (int i=0;i<result_.size();++i) {
        int j = result_[i].oppositeIndex;
        if (i < j) pairs.append(qMakePair(i,j));
    }
    if (pairs.isEmpty()) return {arm1_, arm2_};

    ArmFunctor functor(angles_, pairs, referenceDistance);
    Eigen::LevenbergMarquardt<ArmFunctor> lm(functor);
    Eigen::VectorXd x(2); x[0]=arm1_; x[1]=arm2_;
    lm.minimize(x);
    arm1_ = x[0]; arm2_ = x[1];
    return {arm1_, arm2_};
}

CalibrationResult CalibrationEngine::estimateArmsLeastSquares()
{
    if (result_.isEmpty()) return {arm1_, arm2_};
    int n = angles_.size();
    Eigen::MatrixXd A(2*n,2);
    Eigen::VectorXd b(2*n);
    for (int i=0;i<n;++i) {
        const Angles& angle = angles_[i];
        double alphaRad = angle.alfa * M_PI / 180.0;
        double betaRad  = angle.beta  * M_PI / 180.0;
        const QPointF& p = result_[i].position;
        A(2*i,0) = std::cos(alphaRad);
        A(2*i,1) = std::cos(alphaRad - betaRad - M_PI);
        b(2*i)   = p.x();
        A(2*i+1,0) = std::sin(alphaRad);
        A(2*i+1,1) = std::sin(alphaRad - betaRad - M_PI);
        b(2*i+1)   = p.y();
    }
    Eigen::Vector2d x = A.colPivHouseholderQr().solve(b);
    arm1_ = x[0];
    arm2_ = x[1];
    return {arm1_, arm2_};
}
const QVector<CalibrationPoint>& CalibrationEngine::points() const {
    return result_;
}

DeviationResult computeMaxDeviation(const QVector<Angles>& angles,
                                    const QVector<CalibrationPoint>& points,
                                    double arm1,
                                    double arm2,
                                    double percentage)
{
    qDebug()<<"--computeMaxDeviation--  for arm1=" <<arm1 << "arm2=" <<arm2;
    DeviationResult result;
    double factor = (percentage + 100.0) / 100.0;

    for (int idx = 0; idx < points.size(); ++idx)
    {
        const CalibrationPoint& pt = points[idx];
        int i = pt.index;
        int j = pt.oppositeIndex;

        if (i < 0 || j < 0 || i >= angles.size() || j >= angles.size())
            continue;

        const Angles& a = angles.at(i);
        const Angles& b = angles.at(j);



        double x1 = std::cos(a.alfa * M_PI / 180.0) * arm1 * factor;
        double y1 = std::sin(a.alfa * M_PI / 180.0) * arm1 * factor;
        double x2 = x1 + std::cos((a.alfa - a.beta - 180.0) * M_PI / 180.0) * arm2;
        double y2 = y1 + std::sin((a.alfa - a.beta - 180.0) * M_PI / 180.0) * arm2;

        double x1_op = std::cos(b.alfa * M_PI / 180.0) * arm1 * factor;
        double y1_op = std::sin(b.alfa * M_PI / 180.0) * arm1 * factor;
        double x2_op = x1_op + std::cos((b.alfa - b.beta - 180.0) * M_PI / 180.0) * arm2;
        double y2_op = y1_op + std::sin((b.alfa - b.beta - 180.0) * M_PI / 180.0) * arm2;

        double dist = QLineF(QPointF(x2, y2), QPointF(x2_op, y2_op)).length();
        double diff = std::abs(dist - pt.distanceToOpposite);


        if (diff > result.maxDeviationArm1) {
            result.maxDeviationArm1 = diff;
            result.maxIndexArm1 = idx;
            //qDebug() << " novy dev max arm1 " << diff << " index " << idx;
        }
        if (diff < result.minDeviationArm1) {
            result.minDeviationArm1 = diff;
            result.minIndexArm1 = idx;
            //qDebug() << " novy dev min arm1 " << diff << " index " << idx;
        }
        result.sumDeviationArm1=result.sumDeviationArm1+diff;
        //qDebug()<< " sumdeviation arm1 " << result.sumDeviationArm1 ;
        //---------------------arm2

        x1 = std::cos(a.alfa * M_PI / 180.0) * arm1 ;
        y1 = std::sin(a.alfa * M_PI / 180.0) * arm1 ;
        x2 = x1 + std::cos((a.alfa - a.beta - 180.0) * M_PI / 180.0) * arm2 * factor;
        y2 = y1 + std::sin((a.alfa - a.beta - 180.0) * M_PI / 180.0) * arm2 * factor;

        x1_op = std::cos(b.alfa * M_PI / 180.0) * arm1;
        y1_op = std::sin(b.alfa * M_PI / 180.0) * arm1;
        x2_op = x1_op + std::cos((b.alfa - b.beta - 180.0) * M_PI / 180.0) * arm2 * factor;
        y2_op = y1_op + std::sin((b.alfa - b.beta - 180.0) * M_PI / 180.0) * arm2 * factor;

        dist = QLineF(QPointF(x2, y2), QPointF(x2_op, y2_op)).length();
        diff = std::abs(dist - pt.distanceToOpposite);


        if (diff > result.maxDeviationArm2) {
            result.maxDeviationArm2 = diff;
            result.maxIndexArm2 = idx;
            //qDebug() << " novy dev max arm2 " << diff << " index " << idx;
        }
        if (diff < result.minDeviationArm2) {
            result.minDeviationArm2 = diff;
            result.minIndexArm2 = idx;
            //qDebug() << " novy dev min arm2 " << diff << " index " << idx;
        }
        result.sumDeviationArm2=result.sumDeviationArm1+diff;
        //qDebug()<< " sumdeviation arm2 " << result.sumDeviationArm2 ;
    }

    return result;
}

double CalibrationEngine::totalDeviationFromReference(double referenceDistance) const {
    double totalDeviation = 0.0;

    for (const CalibrationPoint& pt : result_) {
        totalDeviation += std::abs(pt.distanceToOpposite - referenceDistance);
    }

    return totalDeviation;
}

int CalibrationEngine::getBetaMinIndex() const {
    return betaMinIndex_;
}

int CalibrationEngine::getBetaMinOpositIndex() const {
    return betaMinOpositIndex_;
}

int CalibrationEngine::getAlfaMinIndex() const {
    return alfaMinIndex_;
}

int CalibrationEngine::getAlfaMinOpositIndex() const {
    return alfaMinOpositIndex_;
}

double CalibrationEngine::getaverage_dist() const {
    return average_dist_;
}

QPointF CalibrationEngine::getCircleCenter() const {
    return circleCenter_;
}

double CalibrationEngine::getCircleRadius() const {
    return circleRadius_;
}

double CalibrationEngine::getAverageCircleFitError() const {
    return averageCircleFitError_;
}

int CalibrationEngine::getmaxErrorCircleIndex() const {
    return maxErrorIndex_;
}

double CalibrationEngine::getmaxErrorCircle() const {
    return maxErrorCircle_;
}

