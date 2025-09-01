#include "calibrationcontroller.h"

CalibrationController::CalibrationController(double arm1, double arm2)
    : engine_(arm1, arm2) {}

void CalibrationController::setAngles(const QVector<Angles>& angles)
{
    engine_.setAngles(angles);
    engine_.computeOpositPoints();
}

CalibrationResult CalibrationController::calibrate(double referenceDistance)
{
    CalibrationResult res = engine_.optimizeArmsLM(referenceDistance);
    engine_.computeOpositPoints();
    return res;
}

CalibrationResult CalibrationController::calibrateLeastSquares()
{
    return engine_.estimateArmsLeastSquares();
}

const CalibrationEngine& CalibrationController::engine() const
{
    return engine_;
}
