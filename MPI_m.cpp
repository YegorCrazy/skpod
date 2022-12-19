#include <exception>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include "mpi.h"

class Matrix {
public:
  // m for rows number, n for columns number
  int m, n;
  // vector of rows
  std::vector<std::vector<double>> matrix_values;

  std::vector<double>& operator[] (int index) {
    return matrix_values[index];
  }
  const std::vector<double>& operator[] (int index) const {
    return matrix_values[index];
  }
};

class ZeroRowException : std::exception {};

double EPSILON = 0.00000001;

bool doubles_equal(double a, double b = 0) {
  return std::abs(a - b) < EPSILON;
}

void get_input(const std::string& filename,
               Matrix& A) {
  std::ifstream input_file(filename);
  if (!A.matrix_values.empty()) {
    std::cout << "A matrix was not empty while reading input, truncating";
    A.matrix_values.clear();
  }
  input_file >> A.m >> A.n;
  for (auto i = 0; i < A.m; ++i) {
    A.matrix_values.push_back({});
    for (auto j = 0; j < A.n; ++j) {
      double cur_elem;
      input_file >> cur_elem;
      A[i].push_back(cur_elem);
    }
  }
}

void print_matrix(const Matrix& matr) {
  for (const auto& row : matr.matrix_values) {
    for (const auto& elem : row) {
      std::cout << elem << " ";
    }
    std::cout << std::endl;
  }
}

int find_leading_row(const Matrix& A, int start_row, int col) {
  int cur_lead_row = -1;
  int cur_max_abs = 0;
  for (int i = start_row; i < A.m; ++i) {
    if (!doubles_equal(A[i][col]) && (std::abs(A[i][col]) > cur_max_abs)) {
      cur_lead_row = i;
      cur_max_abs = std::abs(A[i][col]);
    }
  }
  if (cur_lead_row == -1) {
    throw ZeroRowException();
  }
  return cur_lead_row;
}

void replace_rows(Matrix& A, int first_row_num, int second_row_num) {
  std::swap(A.matrix_values[first_row_num], A.matrix_values[second_row_num]);
}

std::vector<double> reset_elem_to_zero(std::vector<double>& lead,
                                       std::vector<double>& reset,
                                       int col_num) {
  double koef = reset[col_num] / lead[col_num];
  for (int i = col_num; i < lead.size(); ++i) {
    reset[i] -= lead[i] * koef;
  }
  return reset;
}

int get_proc_num (int row_num, int size) {
  return row_num % (size - 1) + 1;
}

int PREPARE_TO_GET = 10;
int PREPARE_TO_SEND = 11;
int BREAK = 12;

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  bool rank_fl = true, time_fl = true;
  if (argc > 1) {
    if (std::string(argv[1]) == "time") {
      rank_fl = false;
    } else if (std::string(argv[1]) == "rank") {
      time_fl = false;
    }
  }
  int mpi_size, mpi_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  if (mpi_rank == 0) {
    std::string filename = "test_input";
    Matrix A;
    get_input(filename, A);
    double time_start = MPI_Wtime();
    MPI_Bcast(&(A.n), 1, MPI_INT, 0, MPI_COMM_WORLD);
    int cur_col_num = 0;
    int cur_row_num = 0;
    MPI_Status status;
    MPI_Request request;
    for (int i = 0; i < A.m; ++i) {
      cur_row_num = i;
      int leading_row = -1;
      for (int j = cur_col_num; j < A.n; ++j) {
        try {
          leading_row = find_leading_row(A, i, j);
        } catch (ZeroRowException& ex) {
          continue;
        }
        cur_col_num = j;
        break;
      }
      if (leading_row == -1) {
        cur_row_num -= 1;
        break;
      }
      replace_rows(A, i, leading_row);
      for (int j = i + 1; j < A.m; ++j) {
        int proc_num = get_proc_num(j, mpi_size);
        MPI_Isend(&j, 1, MPI_INT, proc_num, PREPARE_TO_GET, MPI_COMM_WORLD, &request);
        MPI_Isend(&(A[cur_row_num][0]), A.n, MPI_DOUBLE, proc_num, 0, MPI_COMM_WORLD, &request);
        MPI_Isend(&(A[j][0]), A.n, MPI_DOUBLE, proc_num, 1, MPI_COMM_WORLD, &request);
        MPI_Isend(&cur_col_num, 1, MPI_INT, proc_num, 2, MPI_COMM_WORLD, &request);
      }
      std::vector<double> new_row;
      new_row.resize(A.n);
      for (int j = i + 1; j < A.m; ++j) {
        int proc_num = get_proc_num(j, mpi_size);
        MPI_Recv(&(new_row[0]), A.n, MPI_DOUBLE, proc_num, j, MPI_COMM_WORLD, &status);
        for (int k = 0; k < A.n; ++k) {
          A[j][k] = new_row[k];
        }
      }
      cur_col_num += 1;
    }
    for (int i = 1; i < mpi_size; ++i) {
      MPI_Isend(&BREAK, 1, MPI_INT, i, BREAK, MPI_COMM_WORLD, &request);
    }
    int rank = cur_row_num + 1;
    double time_finish = MPI_Wtime();
    if (rank_fl) {
      std::cout << rank << std::endl;
    }
    if (time_fl) {
      std::cout << time_finish - time_start << std::endl;
    }
    MPI_Finalize();
  } else {
    int row_len;
    MPI_Status status;
    MPI_Bcast(&row_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    while (true) {
      int row_num;
      MPI_Recv(&row_num, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if (status.MPI_TAG == PREPARE_TO_GET) {
        std::vector<double> lead;
        lead.resize(row_len);
        std::vector<double> reset;
        reset.resize(row_len);
        int col_num;
        MPI_Recv(&(lead[0]), row_len, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&(reset[0]), row_len, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&col_num, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        auto result = reset_elem_to_zero(lead, reset, col_num);
        MPI_Send(&(result[0]), row_len, MPI_DOUBLE, 0, row_num, MPI_COMM_WORLD);
      } else if (status.MPI_TAG == BREAK) {
        break;
      }
    }
    MPI_Finalize();
  }
  return 0;
}
