#include "calibrationengine.h"
#include <QDebug>
#include <algorithm>

struct IndexedPoint {
    QPointF pt;
    int index;
};

static double cross(const QPointF &a, const QPointF &b) {
    return a.x() * b.y() - a.y() * b.x();
}

static double cross(const QPointF &o, const QPointF &a, const QPointF &b) {
    return cross(QPointF(a.x() - o.x(), a.y() - o.y()),
                 QPointF(b.x() - o.x(), b.y() - o.y()));
}

static QVector<IndexedPoint> computeHull(const QVector<IndexedPoint> &points) {
    QVector<IndexedPoint> pts = points;
    std::sort(pts.begin(), pts.end(), [](const IndexedPoint &p1, const IndexedPoint &p2) {
        if (p1.pt.x() == p2.pt.x())
            return p1.pt.y() < p2.pt.y();
        return p1.pt.x() < p2.pt.x();
    });

    QVector<IndexedPoint> hull;
    hull.reserve(pts.size() * 2);

    for (const auto &p : pts) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2].pt, hull[hull.size() - 1].pt, p.pt) <= 0)
            hull.pop_back();
        hull.append(p);
    }
    int lowerSize = hull.size();
    for (int i = pts.size() - 2; i >= 0; --i) {
        const auto &p = pts[i];
        while (hull.size() > lowerSize && cross(hull[hull.size() - 2].pt, hull[hull.size() - 1].pt, p.pt) <= 0)
            hull.pop_back();
        hull.append(p);
    }
    if (hull.size() > 1)
        hull.pop_back();
    return hull;
}

static QVector<int> rotatingCalipers(const QVector<IndexedPoint> &hull) {
    int h = hull.size();
    QVector<int> opp(h, -1);
    if (h < 2) {
        if (h == 1)
            opp[0] = 0;
        return opp;
    }
    int j = 1;
    for (int i = 0; i < h; ++i) {
        int next_i = (i + 1) % h;
        while (std::abs(cross(hull[i].pt, hull[next_i].pt, hull[(j + 1) % h].pt)) >
               std::abs(cross(hull[i].pt, hull[next_i].pt, hull[j].pt)))
            j = (j + 1) % h;
        opp[i] = j;
    }
    return opp;
}

static void fitCirclePratt(const QVector<QPointF> &pts, QPointF &center, double &radius) {
    const int N = pts.size();
    double meanX = 0.0, meanY = 0.0;
    for (const QPointF &p : pts) {
        meanX += p.x();
        meanY += p.y();
    }
    meanX /= N;
    meanY /= N;

    double Suu = 0, Suv = 0, Svv = 0, Suuu = 0, Svvv = 0, Suvv = 0, Svuu = 0;
    for (const QPointF &p : pts) {
        double u = p.x() - meanX;
        double v = p.y() - meanY;
        double uu = u * u;
        double vv = v * v;
        Suu += uu;
        Svv += vv;
        Suv += u * v;
        Suuu += uu * u;
        Svvv += vv * v;
        Suvv += u * vv;
        Svuu += v * uu;
    }

    double A = Suu;
    double B = Suv;
    double C = Svv;
    double D = 0.5 * (Suuu + Suvv);
    double E = 0.5 * (Svvv + Svuu);
    double denom = A * C - B * B;

    if (std::abs(denom) > 1e-12) {
        double uc = (C * D - B * E) / denom;
        double vc = (A * E - B * D) / denom;
        center = QPointF(uc + meanX, vc + meanY);
        radius = std::sqrt(uc * uc + vc * vc + (Suu + Svv) / N);
    } else {
        center = QPointF();
        radius = 0.0;
    }
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
    qDebug()<<"--computeOpositPoints--";
    const double delta = arm1_ * 0.01; // 1% změna pro numerickou derivaci
    const double degToRad = M_PI / 180.0;

    betaMinIndex_ = -1;
    betaMinOpositIndex_ = -1;
    alfaMinIndex_ = -1;
    alfaMinOpositIndex_ = -1;
    double minDiffArm1 = std::numeric_limits<double>::max();
    double minDiffArm2 = std::numeric_limits<double>::max();

    result_.clear();
    result_.reserve(angles_.size());
    for (int i = 0; i < angles_.size(); ++i) {
        const Angles& angle = angles_[i];
        const double alphaRad = angle.alfa * degToRad;
        const double betaRad  = angle.beta * degToRad;
        const double cosAlpha = std::cos(alphaRad);
        const double sinAlpha = std::sin(alphaRad);
        const double cosAlphaBeta = std::cos(alphaRad - betaRad - M_PI);
        const double sinAlphaBeta = std::sin(alphaRad - betaRad - M_PI);

        const double x1 = cosAlpha * arm1_;
        const double y1 = sinAlpha * arm1_;
        const double x2 = x1 + cosAlphaBeta * arm2_;
        const double y2 = y1 + sinAlphaBeta * arm2_;

        CalibrationPoint pt;
        pt.position = QPointF(x2, y2);
        pt.index = i;

        const double x1_up1 = cosAlpha * (arm1_ + delta);
        const double y1_up1 = sinAlpha * (arm1_ + delta);
        const double x2_up1 = x1_up1 + cosAlphaBeta * arm2_;
        const double y2_up1 = y1_up1 + sinAlphaBeta * arm2_;
        pt.diffArm1 = std::hypot(x2 - x2_up1, y2 - y2_up1);

        const double x2_up2 = x1 + cosAlphaBeta * (arm2_ + delta);
        const double y2_up2 = y1 + sinAlphaBeta * (arm2_ + delta);
        pt.diffArm2 = std::hypot(x2 - x2_up2, y2 - y2_up2);

        if (pt.diffArm1 < minDiffArm1) {
            minDiffArm1 = pt.diffArm1;
            betaMinIndex_ = i;
        }
        if (pt.diffArm2 < minDiffArm2) {
            minDiffArm2 = pt.diffArm2;
            alfaMinIndex_ = i;
        }

        result_.append(pt);
       }

    // Fit kružnice metodou Pratt
    QVector<QPointF> positions;
    positions.reserve(result_.size());
    for (const CalibrationPoint &pt : result_)
        positions.append(pt.position);

    QPointF circleCenter;
    double radius = 0.0;
    fitCirclePratt(positions, circleCenter, radius);
    circleCenter_ = circleCenter;
    circleRadius_ = radius;

    double totalCircleError = 0.0;
    maxErrorCircleIndex_ = -1;
    maxErrorCircle_ = 0;
    averageCircleFitError_ = 0;
    for (int i = 0; i < result_.size(); ++i) {
        const QPointF &p = result_[i].position;
        double dist = std::hypot(p.x() - circleCenter.x(), p.y() - circleCenter.y());
        double err = std::abs(dist - radius);
        totalCircleError += err;
        if (err > maxErrorCircle_) {
            maxErrorCircle_ = err;
            maxErrorCircleIndex_ = i;
        }
    }
    averageCircleFitError_ = totalCircleError / result_.size();

    // Najdi protilehlé body pomocí konvexního obalu a rotating calipers
    QVector<IndexedPoint> indexed;
    indexed.reserve(result_.size());
    for (const CalibrationPoint &pt : result_)
        indexed.append({pt.position, pt.index});

    QVector<IndexedPoint> hull = computeHull(indexed);
    QVector<int> hullOpp = rotatingCalipers(hull);
    QVector<bool> onHull(result_.size(), false);
    for (int i = 0; i < hull.size(); ++i) {
        onHull[hull[i].index] = true;
        int oppHullIdx = hullOpp[i];
        int oppOrig = hull[oppHullIdx].index;
        const QPointF &p1 = result_[hull[i].index].position;
        const QPointF &p2 = result_[oppOrig].position;
        double dist = std::hypot(p1.x() - p2.x(), p1.y() - p2.y());
        auto &r = result_[hull[i].index];
        r.opposite = p2;
        r.oppositeIndex = oppOrig;
        r.distanceToOpposite = dist;
        r.diff = dist;
        if (hull[i].index == betaMinIndex_)
            betaMinOpositIndex_ = oppOrig;
        if (hull[i].index == alfaMinIndex_)
            alfaMinOpositIndex_ = oppOrig;
    }
    for (int i = 0; i < result_.size(); ++i) {
        if (onHull[i])
            continue;
        const QPointF &p = result_[i].position;
        double maxDist = 0.0;
        int farIndex = -1;
        for (const auto &hp : hull) {
            const QPointF &hpPos = result_[hp.index].position;
            double d = std::hypot(p.x() - hpPos.x(), p.y() - hpPos.y());
            if (d > maxDist) {
                maxDist = d;
                farIndex = hp.index;
            }
        }
        result_[i].opposite = result_[farIndex].position;
        result_[i].oppositeIndex = farIndex;
        result_[i].distanceToOpposite = maxDist;
        result_[i].diff = maxDist;
        if (i == betaMinIndex_)
            betaMinOpositIndex_ = farIndex;
        if (i == alfaMinIndex_)
            alfaMinOpositIndex_ = farIndex;
    }
    // Výpočet průměrné vzdálenosti
    double sum = 0.0;
    for (const auto& pt : result_) {
        sum += pt.distanceToOpposite;
    }
    double average = sum / result_.size();
    average_dist_ = average;
    // Najdi bod s největší odchylkou od průměru
    double maxError = 0.0;
    maxErrorIndex_ = -1;
    for (int i = 0; i < result_.size(); ++i) {
        double error = std::abs(result_[i].distanceToOpposite - average);
        if (error > maxError) {
            maxError = error;
            maxErrorIndex_ = i;
        }
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

CalibrationResult CalibrationEngine::optimizeArmsLeastSquares(double referenceDistance) {
    qDebug() << "--optimizeArmsLeastSquares--";
    const double delta = 0.001;
    auto computeDistances = [&](double a1, double a2) {
        CalibrationEngine eng(a1, a2);
        eng.setAngles(angles_);
        eng.computeOpositPoints();
        QVector<double> dist;
        for (const CalibrationPoint& pt : eng.points()) {
            dist.append(pt.distanceToOpposite);
        }
        return dist;
    };

    QVector<double> base = computeDistances(arm1_, arm2_);
    QVector<double> dArm1 = computeDistances(arm1_ + delta, arm2_);
    QVector<double> dArm2 = computeDistances(arm1_, arm2_ + delta);

    double a11 = 0.0, a12 = 0.0, a22 = 0.0;
    double b1 = 0.0, b2 = 0.0;
    for (int i = 0; i < base.size(); ++i) {
        double e = base[i] - referenceDistance;
        double der1 = (dArm1[i] - base[i]) / delta;
        double der2 = (dArm2[i] - base[i]) / delta;
        a11 += der1 * der1;
        a12 += der1 * der2;
        a22 += der2 * der2;
        b1 += der1 * e;
        b2 += der2 * e;
    }

    double det = a11 * a22 - a12 * a12;
    double corr1 = 0.0, corr2 = 0.0;
    if (std::abs(det) > 1e-9) {
        corr1 = (-b1 * a22 + b2 * a12) / det;
        corr2 = (-a11 * b2 + a12 * b1) / det;
    }

    return CalibrationResult{arm1_ + corr1, arm2_ + corr2};
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

