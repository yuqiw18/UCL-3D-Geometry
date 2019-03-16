#include <Eigen/Dense>
#include <Eigen/SVD>
#include <Eigen/Sparse>
#include <random>
#include <iostream>
#include <math.h>
#include <time.h>
#include <igl/boundary_loop.h>
#include <igl/bounding_box.h>
#include <igl/vertex_triangle_adjacency.h>
#include <igl/adjacency_list.h>
#include <igl/fit_plane.h>
#include <igl/doublearea.h>
#include <igl/cotmatrix.h>
#include <igl/massmatrix.h>
#include "nanoflann.hpp"
#include "ms.h"

Eigen::SparseMatrix<double> MS::LaplacianMatrix(Eigen::MatrixXd V_in, Eigen::MatrixXi F_in) {
    Eigen::SparseMatrix<double> laplacian_matrix(V_in.rows(), V_in.rows());
    std::vector<std::vector<int>> V_adjacent;
    igl::adjacency_list(F_in, V_adjacent);

    for (int i = 0; i < V_in.rows(); i++){
        int num_adjacent_vertex = V_adjacent[i].size();

        for (int j = 0; j < num_adjacent_vertex; j ++){
            laplacian_matrix.insert(i, V_adjacent[i][j]) = 1.0 / num_adjacent_vertex;
        }

        laplacian_matrix.insert(i,i) = -1.0;
    }

    return laplacian_matrix;
}

Eigen::SparseMatrix<double> MS::CotangentMatrix(Eigen::MatrixXd V_in, Eigen::MatrixXi F_in){

    Eigen::SparseMatrix<double> contangent_matrix(V_in.rows(), V_in.rows());
    std::vector<std::vector<int>> V_adjacent;
    igl::adjacency_list(F_in, V_adjacent, true);
    Eigen::Vector3d V_current, V_1, V_2, V_3;

    for (int i = 0; i < V_adjacent.size(); i++){

        double sum_cotangent = 0;
        V_current = V_in.row(i);

        for (int j = 0; j < V_adjacent[i].size(); j++){
            if (j == 0){
                V_1 = V_in.row(V_adjacent[i][V_adjacent[i].size()-1]);
                V_2 = V_in.row(V_adjacent[i][j]);
                V_3 = V_in.row(V_adjacent[i][j+1]);
            }
            else if (j + 1 == V_adjacent[i].size()){
                V_1 = V_in.row(V_adjacent[i][j-1]);
                V_2 = V_in.row(V_adjacent[i][j]);
                V_3 = V_in.row(V_adjacent[i][0]);
            }else{
                V_1 = V_in.row(V_adjacent[i][j-1]);
                V_2 = V_in.row(V_adjacent[i][j]);
                V_3 = V_in.row(V_adjacent[i][j+1]);
            }

            double cosine_alpha = (V_current - V_1).dot(V_2 - V_1)/((V_current - V_1).norm()*(V_2 - V_1).norm());
            double cosine_beta = (V_current - V_3).dot(V_2 - V_3)/((V_current - V_3).norm()*(V_2 - V_3).norm());
            double sum = 0.5 * (1.0/std::tan(std::acos(cosine_alpha)) + 1.0/std::tan(std::acos(cosine_beta)));

            if (V_in.row(i) != V_in.row(V_adjacent[i][j])){
                contangent_matrix.insert(i, V_adjacent[i][j]) = sum;
                sum_cotangent += sum;
            }

        }
        contangent_matrix.insert(i,i) = -1.0 * sum_cotangent;
    }

    return contangent_matrix;

}

Eigen::SparseMatrix<double> MS::BarycentricMassMatrix(Eigen::MatrixXd V_in, Eigen::MatrixXi F_in){
    Eigen::SparseMatrix<double> area_sparse;

    Eigen::VectorXd area_each_vertex(V_in.rows());

    // Compute the area for each face in the mesh
    Eigen::MatrixXd area_value_list;
    igl::doublearea(V_in, F_in, area_value_list);

    // Find connected faces for each vertex
    std::vector<std::vector<int> > VF;
    std::vector<std::vector<int> > VFi;
    igl::vertex_triangle_adjacency(V_in.rows(), F_in, VF, VFi);
    for (int i = 0; i < VF.size(); i++){
        double total_area = 0;
        std::vector<int> faces = VF[i];
        for (int j = 0; j < faces.size(); j++){
            total_area += area_value_list.row(faces[j])[0];
        }
        area_each_vertex[i] = 0.5 * (total_area/3);
    }

    // Convert the column to diagonal
    Eigen::MatrixXd area_dense(area_each_vertex.asDiagonal());

    // Convert the dense matrix to sparse
    area_sparse = area_dense.sparseView();

    return area_sparse;
}

Eigen::VectorXd MS::UniformMeanCurvature(Eigen::MatrixXd V_in, Eigen::MatrixXi F_in){

    Eigen::VectorXd H(V_in.rows());
    H.setZero();

    Eigen::MatrixXd laplacian_dense = LaplacianMatrix(V_in,F_in).toDense();
    Eigen::MatrixXd laplacian_vertex = laplacian_dense * V_in;

    H = 0.5 * (laplacian_vertex).rowwise().norm();
    return H;
}

Eigen::VectorXd MS::UniformGaussianCurvature(Eigen::MatrixXd V_in, Eigen::MatrixXi F_in){

    Eigen::VectorXd k_each_vertex(V_in.rows());
    Eigen::SparseMatrix<double> area = BarycentricMassMatrix(V_in, F_in);

    std::vector<std::vector<int>> V_adjacent;
    igl::adjacency_list(F_in, V_adjacent, true);

    Eigen::Vector3d V_current, V_1, V_2;

    for (int i = 0; i < V_adjacent.size(); i++){
        double sum_theta = 0;
        V_current = V_in.row(i);
        for (int j = 0; j < V_adjacent[i].size(); j++){

            if (j + 1 == V_adjacent[i].size()){
                V_1 = V_in.row(V_adjacent[i][j]);
                V_2 = V_in.row(V_adjacent[i][0]);
            }else{
                V_1 = V_in.row(V_adjacent[i][j]);
                V_2 = V_in.row(V_adjacent[i][j+1]);
            }

            double theta = std::acos((V_1 - V_current).dot(V_2 - V_current)/((V_1 - V_current).norm() * (V_2 - V_current).norm()));
            sum_theta += theta;
        }
        k_each_vertex[i] = (2*M_PI-sum_theta)/area.coeff(i,i);
    }

    return k_each_vertex;
}

Eigen::VectorXd MS::NonUniformMeanCurvature(Eigen::MatrixXd V_in, Eigen::MatrixXi F_in){

    Eigen::VectorXd H(V_in.rows());
    H.setZero();

    Eigen::SparseMatrix<double> area, cotangent;
    cotangent = CotangentMatrix(V_in, F_in);
    area = BarycentricMassMatrix(V_in, F_in);

    // For testing purpose
//    Eigen::SparseMatrix<double> area_built_in, cotangent_built_in;
//    igl::massmatrix(V_in, F_in, igl::MASSMATRIX_TYPE_BARYCENTRIC, area_built_in);
//    igl::cotmatrix(V_in, F_in, cotangent_built_in);
//    std::cout << "Result(M): Custom vs Built-in Function" << std::endl;
//    std::cout << area.row(0) << std::endl;
//    std::cout << area_built_in.row(0) << std::endl;
//    std::cout << "Result(C): Custom vs Built-in Function" << std::endl;
//    Eigen::VectorXd cot_row1 = cotangent.row(0);
//    Eigen::VectorXd cot_row2 = cotangent_built_in.row(0);
//    for (int i =0; i < cot_row1.size(); i ++){
//        if (cot_row1[i]!=0){
//            std::cout << cot_row1[i] << std::endl;
//        }
//    }
//    std::cout << "-----------------------------------------" << std::endl;
//    for (int i =0; i < cot_row2.size(); i ++){
//        if (cot_row2[i]!=0){
//            std::cout << cot_row2[i] << std::endl;
//        }
//    }

    Eigen::SparseMatrix<double> area_inverse(V_in.rows(), V_in.rows());

    for (int i = 0; i < V_in.rows(); i++)
    {
        area_inverse.insert(i, i) = 1 / area.coeff(i, i);
    }

    Eigen::SparseMatrix<double> sparse_matrix(V_in.rows(), V_in.rows());
    sparse_matrix = area_inverse * cotangent;

    Eigen::MatrixXd dense_matrix = sparse_matrix.toDense();
    Eigen::MatrixXd cotangent_vertex = dense_matrix * V_in;

    H = 0.5 * (cotangent_vertex).rowwise().norm();
    return H;

}

Eigen::MatrixXd MS::AddNoise(Eigen::MatrixXd V_in, double noise){
    
    // Initialise output matrix
    Eigen::MatrixXd V_out;
    V_out.resize(V_in.rows(), V_in.cols());
    V_out.setZero();

    Eigen::MatrixXd V_bounding;
    Eigen::MatrixXi F_bounding;

    igl::bounding_box(V_in, V_bounding, F_bounding);
    double noise_scale_x = abs(V_bounding.row(0).x()-V_bounding.row(4).x());
    double noise_scale_y = abs(V_bounding.row(0).y()-V_bounding.row(2).y());
    double noise_scale_z = abs(V_bounding.row(0).z()-V_bounding.row(1).z());

    // Random number generation
    std::default_random_engine rnd;
    std::normal_distribution<double> gaussian(0.0, noise);
    
    // Add noise to the vertex
    for (int i=0; i<V_out.rows(); i++){
        double x =gaussian(rnd)/(10000*noise_scale_x);
        double y =gaussian(rnd)/(10000*noise_scale_y);
        double z =gaussian(rnd)/(10000*noise_scale_z);
        Eigen::RowVector3d noise(x,y,z);
        V_out.row(i) = V_in.row(i) + noise;
    }
    
    return V_out;
    
}

