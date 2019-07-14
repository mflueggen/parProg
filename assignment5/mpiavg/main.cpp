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
    //TODO root communicator with the first ranks of each precision communicator (to efficiently exchange the results)


    //Get data
    //TODO make it failsave (no guarantees)
    std::ifstream input(data_file);
    std::vector<double> data;
    std::string line;
    while (input >> line)
        data.push_back(std::stod(line));


//        printf("WORLD RANK/SIZE: %d/%d \t ROW RANK/SIZE: %d/%d\n",
//           world_rank, world_size, precision_rank, precision_size);
//        std::cout << data.size() << std::endl;

        int chunk = std::ceil(data.size() / (float) precision_size);

        chunk = precision_rank * chunk + chunk > data.size() ? data.size() - precision_rank * chunk : chunk;
        double sum = 0;
        for (int i = precision_rank * chunk; i < precision_rank * chunk + chunk; ++i) {
            if (is_integer_precision)
                sum += std::floor(data[i]);
            else
                sum += data[i];

        }
        sum /= chunk;
        if (precision_rank == 0)
                std::cout << sum << std::endl;
        // TODO calc integer precision average and use reduce to get the results
        // TODO receive double preciison result.

        //TODO calc double precision average and use reduce as well
        //TODO send result to integer root via root communicator


    MPI_Comm_free(&precision_comm);
    // Finalize the MPI environment.
    MPI_Finalize();
}
