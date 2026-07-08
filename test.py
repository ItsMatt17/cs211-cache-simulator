import sys


MY_PRED = "test123.txt"
ACTUAL = "tests/predictions3.txt"

def main(): 
    with open(MY_PRED) as f:
        s1 = {int(line, 16) for line in f if line.strip()}
    
    with open(ACTUAL) as f:
        s2 = {int(line, 16) for line in f if line.strip()}


    diff = s1.difference(s2)
    for i in diff: 
        print(f"{i}")
    inter = s1.intersection(s2)
    print(f"Inter: {len(inter)}")
    print(f"My Pred: {len(s1)}")
    print(f"Actual Pred {len(s2)}")
    print(f"Hit Accuracy: {len(inter) / len(s2)}")

    
if __name__ == "__main__": 
    main()

