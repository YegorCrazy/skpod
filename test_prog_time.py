import testgen
import subprocess
import os
import math

num_iter = 5
num_proc = 32
filename = 'test_input'
progs = ['normal_algo', 'OpenMP', 'MPI_m']
progs_commands = {
    'normal_algo': ['./normal_algo', filename, 'time'],
    'OpenMP': ['./OpenMP', filename, 'time'],
    'MPI_m': ['mpiexec', '-n', '2', './MPI_m', 'time']
}
prog_env = os.environ.copy()
prog_env['OMP_NUM_THREADS'] = '8'
times = [0 for prog in progs]

for num_threads in [2 ** x for x in range(1, int(math.log2(num_proc) + 1))]:
    for i in range(num_iter):
        testgen.gen_test_file(filename, [400, 402], [400, 402])
        for prog_num in range(len(progs)):
            prog_name = progs[prog_num]
            if prog_name == 'OpenMP':
                prog_env['OMP_NUM_THREADS'] = str(num_threads)
            if prog_name == 'MPI':
                prog_env['OMP_NUM_THREADS'] = '8'
                progs_commands['MPI'][2] = str(num_threads)
                # print(progs_commands['MPI'])
            prog_time = float(subprocess.check_output(progs_commands[prog_name],
                                                      universal_newlines=True,
                                                      env=prog_env))
            times[prog_num] += prog_time
        print(f'iter {i + 1} passed')
    print(f'\nthreads {num_threads} passed')
    times = list(map(lambda x: x / num_iter, times))
    for prog_num in range(len(progs)):
        print(f'{progs[prog_num]}: {times[prog_num]}')
    print()
    times = [0 for prog in progs]

testgen.delete_test_file(filename)


        
        
