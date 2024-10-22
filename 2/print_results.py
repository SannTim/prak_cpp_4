import pandas as pd

# Считываем данные из CSV файла
data = pd.read_csv('results_parallel.csv')

# Группируем данные по количеству потоков и вычисляем среднее значение K
average_quality = data.groupby('thr')['K'].mean().reset_index()

# Выводим результаты в консоль
print("Среднее значение K относительно количества потоков (thr):")
print(average_quality)

