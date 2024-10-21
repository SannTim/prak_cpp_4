import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from tqdm import tqdm

# Загрузка данных
try:
    data = pd.read_csv('results_alg.csv')
except FileNotFoundError:
    print("Файл не найден. Пожалуйста, проверьте имя и путь к файлу.")
    exit()

# Проверка на наличие необходимых столбцов
expected_columns = ['M', 'N', 'boltzmann_time', 'cauchy_time', 'log_cauchy_time']
if not all(col in data.columns for col in expected_columns):
    print("Некоторые ожидаемые столбцы отсутствуют в данных.")
    exit()

def create_heatmap(data, value_col, title, eps_filename):
    heatmap_data = data.pivot_table(index="M", columns="N", values=value_col)
    
    plt.figure(figsize=(10, 8))
    sns.heatmap(heatmap_data, cmap="Reds", annot=False, fmt=".2f", cbar=True)

    plt.title(title)
    plt.xlabel("N (количество работ)")
    plt.ylabel("M (количество процессоров)")
    
    plt.gca().invert_yaxis()  # Инвертируем ось Y
    plt.savefig(eps_filename, format='eps', dpi=300)  # Сохраняем в формате EPS
    plt.close()

# Генерация тепловых карт для каждого алгоритма
for algo, col, title, eps_filename in tqdm([
    ('Boltzmann', 'boltzmann_time', 'Тепловая карта для алгоритма Больцмана', 'boltzmann_heatmap.eps'),
    ('Cauchy', 'cauchy_time', 'Тепловая карта для алгоритма Коши', 'cauchy_heatmap.eps'),
    ('Logarithmic Cauchy', 'log_cauchy_time', 'Тепловая карта для логарифмического алгоритма Коши', 'log_cauchy_heatmap.eps')
]):
    create_heatmap(data, col, title, eps_filename)

print("Тепловые карты сохранены в формате EPS.")

