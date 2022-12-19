import random
import numpy as np
import testgen

tests_num = 100
prog_name = 'MPI'
filename = 'test_input'

def matr_elem_gen():
    return random.choices(
        [0] + [x for x in range(-5, 6) if x != 0],
        weights=[90] + [1 for x in range(10)])

for k in range(tests_num):
    matr, m, n = testgen.gen_test_file(filename,
                                       [1, 10], [1, 10],
                                       matr_elem_gen)
    ans = np.linalg.matrix_rank([np.matrix(matr)])[0]
    prog_ans = int(subprocess.check_output(['mpiexec',
                                            './' + prog_name,
                                            '-n', '4',
                                            'rank'],
                                       universal_newlines=True))
    print(f'Test #{k + 1}, Prog_ans = {prog_ans}, ans = {ans}')
    if ans < min(m, n):
        print('Good test')
    if ans != prog_ans:
        print(m, n, sep=' ')
        for i in range(m):
            print(*matr[i], sep=' ')
        break

testgen.delete_test_file(filename)
                
    
