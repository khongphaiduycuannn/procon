# swagger doc: https://proconvn.duckdns.org/docs

import requests
import json
import sys

url = "https://proconvn.duckdns.org"
token = sys.argv[1]
question_id = int(sys.argv[2])
output_path = sys.argv[3]
headers = {"Authorization": token}

if len(sys.argv) != 0 and sys.argv[0].isdigit():
    question_id = int(sys.argv[0])

# request steps from file
print("Getting steps from", output_path)
with open(output_path, "r") as file:
    steps = json.loads(file.read())

# send your answer to server
print("Submiting answer to server")
payload = {"question_id": question_id, "answer_data": steps}
res = requests.post(f"{url}/answer", json=payload, headers=headers).json()

#  get your answer id
answer_id = res["id"]

# get your answer and score from server
answer = requests.get(f"{url}/answer/{answer_id}", headers=headers).json()
score_data = json.loads(answer["score_data"])
print("final score:", score_data["final_score"])