import sys

import AWSCTDGenerateImg
import ntpath

def plot_cm(cm, sModel, nClassCount, sDataFile, sWorkingDir):
	file=sWorkingDir+'CM/CM_'+sModel+'_'
	file+=ntpath.basename(sDataFile)
	file+='.svg'
	classes = ['a', 'b', 'c', 'd', 'e', 'f'] if nClassCount != 2 else ['Benign', 'Malware']
	AWSCTDGenerateImg.plot_confusion_matrix(cm, classes, file=file)
