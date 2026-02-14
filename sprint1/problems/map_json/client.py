import requests

# Базовый URL вашего сервера
BASE_URL = 'http://127.0.0.1:8080'

def get_map(map_id):
    # Формируем полный URL с конкретным ID карты
    url = f"{BASE_URL}/api/v1/maps/{map_id}"
    headers = {
        'Content-Type': 'application/json'
    }
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        description = response.json()
        print(f"Описание карты {map_id}:")
        print(description)
    else:
        print(f"Ошибка при запросе карты {map_id}: {response.status_code}")

# Пример использования
if __name__ == "__main__":
    map_id = 'map1'  # замените на нужный ID карты
    get_map(map_id)