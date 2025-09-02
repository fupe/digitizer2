#ifndef CALIBRATIONENGINE_H
#define CALIBRATIONENGINE_H
#include <QVector>
#include <QPointF>
#include <QLineF>
#include <qmath.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>



struct Angles {
    double alfa;
    double beta;
    Angles(double a = 0.0, double b = 0.0) : alfa(a), beta(b) {}
};



struct CalibrationPoint {
    QPointF position;
    QPointF opposite;
    int index = -1;
    int oppositeIndex = -1;
    double distanceToOpposite = 0.0;
    double diff = 0.0;
    double diffArm1 = 0.0;
    double diffArm2 = 0.0;
};

struct CalibrationResult {
    double adjustedArm1;
    double adjustedArm2;
    double rmse = 0.0;
    double maxResidual = 0.0;
    double radiusDiff = 0.0;
    double scaleFactor = 1.0;
};

struct DeviationResult {
    double maxDeviationArm1 = 0.0;
    double minDeviationArm1 = std::numeric_limits<double>::max();
    int maxIndexArm1 = -1;
    int minIndexArm1 = -1;
    double sumDeviationArm1 = 0;

    double maxDeviationArm2 = 0.0;
    double minDeviationArm2 = std::numeric_limits<double>::max();
    int maxIndexArm2 = -1;
    int minIndexArm2 = -1;
    double sumDeviationArm2 = 0;

    // ✨ Přidaná metoda pro výpočet průměrné odchylky
    double averageDeviation() const {
        return (sumDeviationArm1 + sumDeviationArm2) / 2.0;
    }
};



class CalibrationEngine {
public:
    CalibrationEngine(double arm1, double arm2);  //konstruktor
    const QVector<Angles>& getAngles() const;
    CalibrationResult optimizeArmsGrid(double referenceDistance,
                                       double stepArm1,
                                       double stepArm2,
                                       int steps);
    double totalDeviationFromReference(double referenceDistance) const;
    CalibrationResult optimizeArms(double referenceDistance,
                                   double stepArm1,
                                   double stepArm2,
                                   int steps,
                                   int betaMinIndex,
                                   int betaMinOpositIndex,
                                   int alfaMinIndex,
                                   int alfaMinOpositIndex);
    CalibrationResult optimizeArmsLeastSquares(double referenceDistance);
    void setAngles(const QVector<Angles>& angles);  //nastavi sadu uhlu
    void computeOpositPoints(double refRadius = 0.0, bool useRANSAC = false);  //najde protilehle body a jejich indexy
    CalibrationResult evaluateCircleFit(const QPointF& refCenter, double refRadius) const;

    const QVector<CalibrationPoint>& points() const;
    double getArm1();
    double getArm2();
    int getBetaMinIndex() const;
    int getBetaMinOpositIndex() const;
    int getAlfaMinIndex() const;
    int getAlfaMinOpositIndex() const;
    int getMaxErrorIndex() const;
    double getaverage_dist () const;
    QPointF getCircleCenter() const;
    double getCircleRadius() const;
    double getAverageCircleFitError() const;
    int getmaxErrorCircleIndex() const;
    double getmaxErrorCircle() const;
    double getRadiusDiff() const;
    double getScaleFactor() const;

private:
    double arm1_;
    double arm2_;
    QVector<Angles> angles_;
    QVector<CalibrationPoint> result_;
    int betaMinIndex_;
    int betaMinOpositIndex_;
    int alfaMinIndex_;
    int alfaMinOpositIndex_;
    int maxErrorIndex_;
    double average_dist_;
    double sumError_ = 0;
    QPointF circleCenter_;
    double circleRadius_ = 0.0;
    double averageCircleFitError_ = 0.0;
    int maxErrorCircleIndex_ = -1;
    double maxErrorCircle_ = 0;
    double radiusDiff_ = 0.0;
    double scaleFactor_ = 1.0;


};


DeviationResult computeMaxDeviation(const QVector<Angles>& angles,
                           const QVector<CalibrationPoint>& points,
                           double arm1,
                           double arm2,
                           double percentage);

#endif // CALIBRATIONENGINE_H
