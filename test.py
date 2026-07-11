import sys
'''
Baseline CacheInput1
    Inter: 222
    My Pred: 888
    Actual Pred 416
    Hit Accuracy: 0.5336538461538461

Baseline CacheInput2
    Inter: 227
    My Pred: 908
    Actual Pred 421
    Hit Accuracy: 0.5391923990498813
Baseline CacheInput3
    Inter: 616
    My Pred: 872
    Actual Pred 872
    Hit Accuracy: 0.7064220183486238

'''

MY_PRED = "test123.txt"
ACTUAL = "tests/predictions.txt"

def main(): 
    with open(MY_PRED) as f:
        s1 = {int(line, 16) for line in f if line.strip()}
    
    with open(ACTUAL) as f:
        s2 = {int(line, 16) for line in f if line.strip()}


    # diff = s1.difference(s2)
    # for i in diff: 
    #     print(f"{i}")
    inter = s1.intersection(s2)
    print(f"Inter: {len(inter)}")
    print(f"My Pred: {len(s1)}")
    print(f"Actual Pred {len(s2)}")
    print(f"Hit Accuracy: {len(inter) / len(s2)}")

    
if __name__ == "__main__": 
    main()

