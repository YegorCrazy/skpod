import testgen
import subprocess

num_iter = 5
progs = ['normal_algo', 'OpenMP']
times = [0 for prog in progs]
filename = 'testing_file'

for i in range(num_iter):
    testgen.gen_test_file(filename, [500, 1000], [500, 1000])
    for prog_num in range(len(progs)):
        prog_time = float(subprocess.check_output(['./' + progs[prog_num],
                                                filename,
                                                'time'],
                                               universal_newlines=True))
        times[prog_num] += prog_time
    print(f'iter {i + 1} passed')

times = list(map(lambda x: x / num_iter, times))
for prog_num in range(len(progs)):
    print(f'{progs[prog_num]}: {times[prog_num]}')
testgen.delete_test_file(filename)


        
        
