import pandas as pd
import matplotlib.pyplot as plt

# Считываем данные из CSV файла
data = pd.read_csv('results_parallel_time.csv')

# Фильтруем данные для двух наборов
data_large = data[(data['N'] == 100000) & (data['M'] == 5000)]
data_small = data[(data['N'] == 100) & (data['M'] == 10)]

# Подготовка данных для графиков
threads_large = data_large['thr']
time_large = data_large['time']
quality_large = data_large['K']

threads_small = data_small['thr']
time_small = data_small['time']
quality_small = data_small['K']

# Создаем графики
plt.figure(figsize=(12, 10))

# График 1: Время выполнения для большого набора
plt.subplot(2, 2, 1)
plt.plot(threads_large, time_large, marker='o')
plt.title('Время выполнения от количества потоков (N=100000, M=5000)')
plt.xlabel('Количество потоков')
plt.ylabel('Время (с)')
plt.grid(True)

# График 2: Качество для большого набора
plt.subplot(2, 2, 2)
plt.plot(threads_large, quality_large, marker='o', color='orange')
plt.title('Качество от количества потоков (N=100000, M=5000)')
plt.xlabel('Количество потоков')
plt.ylabel('Качество')
plt.grid(True)

# График 3: Время выполнения для малого набора
plt.subplot(2, 2, 3)
plt.plot(threads_small, time_small, marker='o')
plt.title('Время выполнения от количества потоков (N=100, M=10)')
plt.xlabel('Количество потоков')
plt.ylabel('Время (с)')
plt.grid(True)

# График 4: Качество для малого набора
plt.subplot(2, 2, 4)
plt.plot(threads_small, quality_small, marker='o', color='orange')
plt.title('Качество от количества потоков (N=100, M=10)')
plt.xlabel('Количество потоков')
plt.ylabel('Качество')
plt.grid(True)

plt.tight_layout()
plt.savefig('results_parallel_time.eps', format='eps')
