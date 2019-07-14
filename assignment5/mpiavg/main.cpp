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

    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    std::string data_file = argv[1];
    int integer_precision_size = std::stoi(argv[2]);
    int double_precision_size = std::stoi(argv[3]);


    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    bool is_integer_precision = world_rank / double_precision_size;
    int color = is_integer_precision ? 1 : 0; // split ranks into integer and double precision group

// Split the communicator based on the color and use the
// original rank for ordering
    MPI_Comm precision_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &precision_comm);
    int precision_rank, precision_size;
    MPI_Comm_rank(precision_comm, &precision_rank);
    MPI_Comm_size(precision_comm, &precision_size);

    color = precision_rank == 0 ? 1 : 0;
    MPI_Comm root_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &root_comm);


    //Get data
    //TODO make it failsave (no guarantees)
    std::ifstream input(data_file);
    std::vector<double> data;
    std::string line;
    while (input >> line)
        data.push_back(std::stod(line));


    int global_chunk = std::ceil(data.size() / (float) precision_size);

    int local_chunk = precision_rank * global_chunk + global_chunk > data.size() ? data.size() - precision_rank * global_chunk : global_chunk;
    double sum = 0;
    for (int i = precision_rank * global_chunk; i < precision_rank * global_chunk + local_chunk; ++i) {
        if (is_integer_precision)
            sum += std::floor(data[i]);
        else
            sum += data[i];
    }


    // Reduce all of the local averages into the global average
    double global_avg;
    MPI_Reduce(&sum, &global_avg, 1, MPI_DOUBLE, MPI_SUM, 0,
               precision_comm);
    global_avg /= data.size();


    if (precision_rank == 0) {
        // Gather both averages on the global root process
        double averages[2]; //we only have two rank in the root communicator
        MPI_Gather(&global_avg, 1, MPI_DOUBLE, &averages, 1,
                   MPI_DOUBLE, 0, root_comm);

        if (world_rank == 0) {
            std::cout << averages[1] << std::endl;
            std::cout << averages[0] << std::endl;
        }
    }


    MPI_Comm_free(&precision_comm);
    // Finalize the MPI environment.
    MPI_Finalize();
}
