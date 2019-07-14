#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>

int main(int argc, char** argv) {
    // use fixed 6 digit precision for output
    std::cout << std::fixed;
    std::cout << std::setprecision(6);

    // Parse arguments
    std::string data_file = argv[1];
    int integer_precision_size = std::stoi(argv[2]);
    int double_precision_size = std::stoi(argv[3]);

    // Initialize the MPI environment and all communicators
    MPI_Init(NULL, NULL);

    int world_size;
    int world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Comm precision_comm;
    int precision_rank, precision_size;
    bool is_integer_precision = world_rank / double_precision_size;
    int color = is_integer_precision ? 1 : 0; // split ranks into integer and double precision group. we assume that world_size == integer_precision_size + double_precision_size
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &precision_comm);
    MPI_Comm_rank(precision_comm, &precision_rank);
    MPI_Comm_size(precision_comm, &precision_size);

    MPI_Comm root_comm;
    color = precision_rank == 0 ? 1 : 0;
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &root_comm);


    //Get data, broadcast data between ranks if necessary
    std::ifstream input(data_file);
    std::vector<double> data;
    std::string line;
    while (input >> line)
        data.push_back(std::stod(line));
    int size_report[world_size];
    int data_size = data.size();

    MPI_Allgather(&data_size, 1, MPI_INT, size_report, 1, MPI_INT, MPI_COMM_WORLD);

    bool distribute_data = false;
    int data_source_rank = -1;
    for (int i = world_size - 1; i >= 0 ; --i) {
        if (size_report[i])
            data_source_rank = i;
        else
            distribute_data = true;
    }
    if (data_source_rank == -1) {
        std::cerr << "Input data not available to any rank." << std::endl;
        exit(-1);
    }
    if (distribute_data) {
        if (world_rank != data_source_rank)
            data.resize(size_report[data_source_rank]);
        
        // We do no split up the data since
        // 1. We are not allowed to use p2p mechanisms
        // 2. the data should be accessible by all ranks anyways if the data size has a significant impact on the broadcast.
        MPI_Bcast(data.data(), size_report[data_source_rank], MPI_DOUBLE, data_source_rank, MPI_COMM_WORLD);
    }


    // Each rank calculates the sum of its input fraction
    int global_chunk = std::ceil(data.size() / (float) precision_size);
    int local_chunk = precision_rank * global_chunk + global_chunk > data.size() ? data.size() - precision_rank * global_chunk : global_chunk;
    double sum = 0;
    for (int i = precision_rank * global_chunk; i < precision_rank * global_chunk + local_chunk; ++i) {
        if (is_integer_precision)
            sum += std::floor(data[i]);
        else
            sum += data[i];
    }


    // Reduce all of the local sums into the global sum and calculate average
    double global_avg;
    MPI_Reduce(&sum, &global_avg, 1, MPI_DOUBLE, MPI_SUM, 0,
               precision_comm);
    global_avg /= data.size();


    // Gather both averages on the global root process
    if (precision_rank == 0) {
        double averages[2]; //we only have two rank in the root communicator
        MPI_Gather(&global_avg, 1, MPI_DOUBLE, &averages, 1,
                   MPI_DOUBLE, 0, root_comm);

        if (world_rank == 0) {
            std::cout << averages[1] << std::endl;
            std::cout << averages[0] << std::endl;
        }
    }


    MPI_Comm_free(&root_comm);
    MPI_Comm_free(&precision_comm);
    MPI_Finalize();
}
