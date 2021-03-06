// Global static functions

namespace ICP{

    Eigen::MatrixXd GetSubsample(Eigen::MatrixXd V_to_process, double subsample_rate);

    Eigen::MatrixXd GetVertexNormal(Eigen::MatrixXd V_target);

    std::pair<Eigen::MatrixXd, Eigen::MatrixXd> FindCorrespondences(Eigen::MatrixXd V_target, Eigen::MatrixXd V_to_process);

    std::pair<double, std::pair<Eigen::Matrix3d, Eigen::RowVector3d>> EstimateRigidTransform(Eigen::MatrixXd V_matched, Eigen::MatrixXd V_to_process);

    std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, Eigen::MatrixXd> FindCorrespondencesNormalBased(Eigen::MatrixXd V_target, Eigen::MatrixXd V_to_process, Eigen::MatrixXd N_target);

    std::pair<Eigen::Matrix3d, Eigen::RowVector3d> EstimateRigidTransformNormalBased(Eigen::MatrixXd V_matched, Eigen::MatrixXd V_to_process, Eigen::MatrixXd N_to_process);

    Eigen::MatrixXd ApplyRigidTransform(Eigen::MatrixXd V_to_process, std::pair<Eigen::Matrix3d, Eigen::RowVector3d> transform);
    
    std::pair<Eigen::MatrixXi, Eigen::MatrixXi> FindNonOverlappingFaces(Eigen::MatrixXd V_target, Eigen::MatrixXd V_to_process, Eigen::MatrixXi F_to_process);

    Eigen::MatrixXd Rotate(Eigen::MatrixXd V_in, double x, double y, double z);
    
    Eigen::MatrixXd AddNoise(Eigen::MatrixXd V_in, double sd);

    Eigen::MatrixXd ICPOptimised(Eigen::MatrixXd V_target, Eigen::MatrixXd V_to_process, double subsample_rate);

    Eigen::MatrixXd ICPNormalBased(Eigen::MatrixXd V_target, Eigen::MatrixXd V_to_process);

    Eigen::MatrixXd FindBestStartRotation(Eigen::MatrixXd V_target, Eigen::MatrixXd V_to_process);

    double GetErrorMetric(Eigen::MatrixXd V_target, Eigen::MatrixXd V_to_process);
}

