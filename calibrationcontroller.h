#ifndef CALIBRATIONCONTROLLER_H
#define CALIBRATIONCONTROLLER_H

#include <QVector>
#include "calibrationengine.h"

class CalibrationController {
public:
    CalibrationController(double arm1, double arm2);

    void setAngles(const QVector<Angles>& angles);
    CalibrationResult calibrate(double referenceDistance);
    CalibrationResult calibrateLeastSquares();

    const CalibrationEngine& engine() const;

private:
    CalibrationEngine engine_;
};

#endif // CALIBRATIONCONTROLLER_H
