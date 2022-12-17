import random
import subprocess

def matr_elem_gen():
    return random.choices(
        [0] + [x for x in range(-5, 6) if x != 0],
        weights=[90] + [1 for x in range(10)])

def gen_test_file(filename, m_range=[100, 200],
                  n_range=[100, 200], elem_gen=matr_elem_gen):
    with open(filename, 'w') as file:
        m = random.randint(*m_range)
        file.write(str(m) + ' ')
        n = random.randint(*n_range)
        file.write(str(n) + '\n')
        matr = []
        for i in range(m):
            matr.append([])
            for j in range(n):
                new_elem = matr_elem_gen()[0]
                matr[i].append(new_elem)
                file.write(str(new_elem) + ' ')
            file.write('\n')
        file.flush()
    return (matr, m, n)

def delete_test_file(filename):
    subprocess.run(['rm', filename])

if __name__ == '__main__':
    filename = 'test_input'
    gen_test_file(filename)
