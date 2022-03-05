import requests


class http_sending:
  def __init__(self):
    pass

  def send(self, address, data, headers):
    res = requests.post(address, data=data, headers=headers, timeout=5)