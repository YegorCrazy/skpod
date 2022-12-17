#include <exception>
#include <iostream>
#include <fstream>
#include <vector>
#include <exception>

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

void reset_elem_to_zero(Matrix& A, int leading_row_num, int reset_row_num, int col_num) {
  double koef = A[reset_row_num][col_num] / A[leading_row_num][col_num];
  for (int i = col_num; i < A.n; ++i) {
    A[reset_row_num][i] -= A[leading_row_num][i] * koef;
  }
}

int main(int argc, char** argv) {
  std::string filename;
  if (argc > 1) {
    filename = argv[1];
  } else {
    std::cout << "Input file not specified!" << std::endl;
    return 0;
  }
  bool rank_fl = true, time_fl = true;
  if (argc > 2) {
    if (std::string(argv[2]) == "time") {
      rank_fl = false;
    } else if (std::string(argv[2]) == "rank") {
      time_fl = false;
    } else {
      std::cout << "Output type not specified!" << std::endl;
      return 0;
    }
  }
  Matrix A;
  get_input(filename, A);
  int cur_col_num = 0;
  int cur_row_num = 0;
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
      reset_elem_to_zero(A, cur_row_num, j, cur_col_num);
    }
    cur_col_num += 1;
  }
  int rank = cur_row_num + 1;
  if (rank_fl) {
    std::cout << rank << std::endl;
  }
  if (time_fl) {
    
  }
  return 0;
}