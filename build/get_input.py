# swagger doc: https://proconvn.duckdns.org/docs

import requests
import json
import sys

url = "https://proconvn.duckdns.org"
token = sys.argv[1]
question_id = int(sys.argv[2])
input_path = sys.argv[3]
headers = {"Authorization": token}


if len(sys.argv) != 0 and sys.argv[0].isdigit():
    question_id = int(sys.argv[0])

# request question from server
print(f"Requesting question {question_id}")
question = json.loads(requests.get(f"{url}/question/{question_id}", headers=headers).json()["question_data"])

# reformat question
print("Formating")
for i in range(len(question["board"]["start"])):
    question["board"]["start"][i] = "".join(map(str, question["board"]["start"][i]))
for i in range(len(question["board"]["goal"])):
    question["board"]["goal"][i] = "".join(map(str, question["board"]["goal"][i]))
for i in range(len(question["general"]["patterns"])):
    for j in range(len(question["general"]["patterns"][i]["cells"])):
        question["general"]["patterns"][i]["cells"][j] = "".join(map(str, question["general"]["patterns"][i]["cells"][j]))

# write to file
print("Writing to", input_path)
with open(input_path, "w") as file:
    json.dump(question, file, indent=4, ensure_ascii=False)

print("Done get question!")

