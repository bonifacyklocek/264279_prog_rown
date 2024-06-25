#include <mpi.h>
#include <iostream>
#include <vector>
#include <climits>
#include <random>

using namespace std;

struct Job {
    int id;
    vector<int> processing_times;
};

int calculate_makespan(const vector<Job>& jobs, const vector<int>& job_order) {
    int num_jobs = jobs.size();
    int num_machines = jobs[0].processing_times.size();
    vector<vector<int>> completion_time(num_jobs, vector<int>(num_machines, 0));

    for (int job_index = 0; job_index < num_jobs; ++job_index) {
        int job_id = job_order[job_index];
        for (int machine_index = 0; machine_index < num_machines; ++machine_index) {
            int prev_job_completion = (job_index == 0) ? 0 : completion_time[job_index - 1][machine_index];
            int prev_machine_completion = (machine_index == 0) ? 0 : completion_time[job_index][machine_index - 1];
            completion_time[job_index][machine_index] = max(prev_job_completion, prev_machine_completion) + jobs[job_id].processing_times[machine_index];
        }
    }

    return completion_time[num_jobs - 1][num_machines - 1];
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        cerr << "Brak dostatecznej liczby argumentow" << std::endl;
        return 1;
    }

    int n_times = stoi(argv[1]); // 8
    int n_jobs = stoi(argv[2]); // 4

    vector<Job> jobs;

    vector<int>job_temp(n_times * n_jobs);
    if(rank==0) {
        std::random_device dev;
        std::mt19937 rand(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist15(1, 15); // distribution in range [1, 6]
        for (int i = 0; i < n_jobs; i++) {
            for (int j = 0; j < n_times; j++) {
                job_temp[i*n_times + j] = (int) dist15(rand);
            }
        }
//        for (int i = 0; i < job_temp.size(); i++) {
//            cout <<job_temp[i] << " ";
//        }
        for(int i = 1; i < size; i++){
            MPI_Send(job_temp.data(), job_temp.size(), MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }else{
        MPI_Recv(job_temp.data(), job_temp.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for (int i = 0; i < n_jobs; i++) {
        vector<int>temp(n_times);
        for (int j = 0; j < n_times; j++) {
            temp[j] = job_temp[i*n_times + j];
        }
        jobs.push_back({i, temp});
    }

//    n_jobs = jobs.size();

    vector<int> job_order(n_jobs);
    for (int i = 0; i < n_jobs; ++i) {
        job_order[i] = i;
    }

    int total_permutations = 1;
    for (int i = 1; i <= n_jobs; ++i) {
        total_permutations *= i;
    }
    int permutations_per_process = total_permutations / size;
    int remainder = total_permutations % size;

    int start_permutation = rank * permutations_per_process;
    int end_permutation = start_permutation + permutations_per_process;
    if (rank == size - 1) {
        end_permutation += remainder;
    }

    for (int i = 0; i < start_permutation; ++i) {
        next_permutation(job_order.begin(), job_order.end());
    }

    int local_min_makespan = INT_MAX;
    vector<int> local_best_order;

    for (int i = start_permutation; i < end_permutation; ++i) {
        int current_makespan = calculate_makespan(jobs, job_order);
        if (current_makespan < local_min_makespan) {
            local_min_makespan = current_makespan;
            local_best_order = job_order;
        }
        next_permutation(job_order.begin(), job_order.end());
    }

    int global_min_makespan;
    MPI_Reduce(&local_min_makespan, &global_min_makespan, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "Minimalny czas wytwarzania: " << global_min_makespan << endl;
    }

    MPI_Finalize();
    return 0;
}