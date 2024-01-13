import os
import struct
# received_tmp
# project2_bits_receiver
inputFileName = "./project2_bits_receiver.txt"
fOurOutput = open(inputFileName, mode = "r", encoding="utf-8")
fGroundTruth = open("./INPUT_bin.txt", mode = "r", encoding="utf-8")
# diff result
fOut = open("./diff2.txt", mode = "w", encoding="utf-8")

groundTruth = fGroundTruth.readlines()[0]
our = fOurOutput.readlines()[0]

print(len(groundTruth))
print(len(our))
cntCorrect = 0
for i in range(len(groundTruth)):
    # print(i)
    if i >= len(our):
        fOut.write("1")
    else:
        if groundTruth[i] == our[i]:
            fOut.write("0")
            cntCorrect += 1
        else:
            fOut.write("1")
fOut.close()
fOut = open("diff2.txt", mode = "r+", encoding="utf-8")
old = fOut.read()
fOut.seek(0)
fOut.write("correct: " + str(cntCorrect/ len(groundTruth)) + "\n")
fOut.write(old)

fOurOutput.close()
fGroundTruth.close()
fOut.close()