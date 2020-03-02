import sys
# insert at 1, 0 is the script path (or '' in REPL)
sys.path.insert(1, 'Utils')
if len(sys.argv) != 3 : 
	print("Parameters example: AWSCTD.py file_to_data.csv CNN")
	quit()

import os
import tensorflow as tf

#from keras.backend.tensorflow_backend import set_session
#config = tf.ConfigProto()
#config.gpu_options.allow_growth = True
#config.gpu_options.visible_device_list = ""
#set_session(tf.Session(config=config))

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3' 

m_sDataFile = sys.argv[1]
m_sModel    = sys.argv[2]

import numpy as np
# fix random seed for reproducibility
np.random.seed(0)

import AWSCTDReadData
import AWSCTDCreateModel
import AWSCTDClearSesion
import gc

import os
m_sWorkingDir = os.getcwd()
m_sWorkingDir = m_sWorkingDir + '/'
print m_sWorkingDir

import ConfigParser
config = ConfigParser.ConfigParser()
config.read(m_sWorkingDir+'config.ini')
nEpochs = config.getint('MAIN', 'nEpochs')  # Default 200
nBatchSize = config.getint('MAIN', 'nBatchSize') # Default 100
nPatience = config.getint('MAIN', 'nPatience') # Default 3
nKFolds = config.getint('MAIN', 'nKFolds') # Default 5
bCategorical = config.getboolean('MAIN', 'bCategorical') # Default false

fIniFile = open(m_sWorkingDir+'config.ini', "r")
sConfig = fIniFile.read()
print "Config file:"
print sConfig

m_nParametersCount = 0
m_nClassCount = 0
m_nWordCount = 0
Xtr, Ytr, m_nParametersCount, m_nClassCount, m_nWordCount = AWSCTDReadData.ReadDataImpl(m_sDataFile, bCategorical)
print Ytr
gc.collect()

import math

def step_decay(epoch):
	initial_lrate = 0.01
	drop = 0.8
	epochs_drop = 100.0
	#lrate = initial_lrate * math.pow(drop, math.floor((1+epoch)/epochs_drop))
	lrate = initial_lrate * 1.0 / (1.0 + (0.0000005 * epoch * 100))
	if lrate < 0.0001 :
		return 0.0001
	return lrate

#Calback function to achieve early stopping
from keras.callbacks import EarlyStopping, LearningRateScheduler
sMonitor = 'acc'

if bCategorical:
	sMonitor = 'categorical_accuracy'

#if m_nClassCount == 2:
#	sMonitor = 'acc'
lrate = LearningRateScheduler(step_decay, verbose=1)
callbacks_list = [EarlyStopping(monitor=sMonitor, patience=nPatience, verbose=min)]
#callbacks_list = [lrate]

arrAcc  = []
arrLoss = []
arrMae  = []
arrTimeFit = []
arrTimeTest = []
arrTimePredict = []
from sklearn.model_selection import KFold
kfold = KFold(n_splits=nKFolds, shuffle=True)

from time import gmtime, strftime
sTime = strftime("%Y-%m-%d %H:%M:%S", gmtime())
import time
start = time.time()
nFoldNumber = 1

nAllSize = 0

import AWSCTDPlotAcc
#save the model history in a list after fitting so that we can plot later
model_history = [] 

# For Confusion Matrix
import AWSCTDPlotCM
from sklearn.metrics import confusion_matrix
cm = np.zeros((m_nClassCount,m_nClassCount), dtype=int)

# For ROC
import matplotlib.pyplot as plt
from keras.utils import np_utils
from sklearn.metrics import roc_curve, auc
from scipy import interp
from collections import defaultdict

plt.rcParams['svg.fonttype'] = 'none'

tprs = {}
aucs = {}
EER  = {}

if m_nClassCount == 5:
	tprs = {0:[], 1:[], 2:[], 3:[], 4:[]}
	aucs = {0:[], 1:[], 2:[], 3:[], 4:[]}
	EER  = {0:[], 1:[], 2:[], 3:[], 4:[]}
elif m_nClassCount == 6:
	tprs = {0:[], 1:[], 2:[], 3:[], 4:[], 5:[]}
	aucs = {0:[], 1:[], 2:[], 3:[], 4:[], 5:[]}
	EER  = {0:[], 1:[], 2:[], 3:[], 4:[], 5:[]}
elif m_nClassCount == 2:
	tprs = {0:[], 1:[]}
	aucs = {0:[], 1:[]}
	EER  = {0:[], 1:[]}

mean_fpr = np.linspace(0, 1, 100)

for train, test in kfold.split(Xtr, Ytr):
	print("KFold number: " + str(nFoldNumber))
	nFoldNumber += 1 
	# Create model
	model = AWSCTDCreateModel.CreateModelImpl(m_sModel, m_nWordCount, m_nClassCount, m_nParametersCount, bCategorical)
	startFit = time.time()
	# Train
	history = model.fit([Xtr[train]],Ytr[train],epochs=nEpochs,batch_size=nBatchSize, callbacks=callbacks_list, verbose=1)
	endFit = time.time()
	model_history.append(history)
	tmExecFit = endFit - startFit
	startTest = time.time()
	scores = model.evaluate([Xtr[test]], Ytr[test])
	endTest = time.time()
	
	startPredict = time.time()
	y_pred=model.predict([Xtr[test]])
	endPredict = time.time()
	
	nAllSize = nAllSize + len(Xtr[test])
	
	tmExecTest = endTest - startTest
	arrTimeFit.append(tmExecFit)
	arrTimeTest.append(tmExecTest)
	
	tmExecPredict = endPredict - startPredict
	arrTimePredict.append(tmExecPredict)
	print(model.metrics_names)
	print("%s: %.2f%%" % (model.metrics_names[1], scores[1]*100))
	arrAcc.append(scores[1] * 100)
	arrLoss.append(scores[0])
	
	# To get accuracy comparison with metrics
	nPredCorr = 0
	dAccuracy = 0.0
	
	if bCategorical:
		all_count = len(y_pred)
		for i in range(all_count):
			if np.argmax(Ytr[test][i])==np.argmax(y_pred[i]) :
				nPredCorr += 1
		dAccuracy = float(nPredCorr)/float(all_count)
	else:
		y_pred_class = (y_pred > 0.5)
		y_pred_class = y_pred_class.astype(int)
		all_count = len(y_pred_class)
		for i in range(all_count):
			if Ytr[test][i] == y_pred_class[i] :
				nPredCorr += 1
		dAccuracy = float(nPredCorr)/float(all_count)

	dAccuracy = dAccuracy*100
	print("Accuracy (Sanity Check): %.2f%%" % dAccuracy)

	# ROC calculations
	if bCategorical:
		for x in range(m_nClassCount):
			plt.figure(x)
			fpr, tpr, thresholds = roc_curve(Ytr[test][:, x], y_pred[:, x])
			fnr = 1 - tpr
			eer_ = fpr[np.nanargmin(np.absolute((fnr - fpr)))]
			EER[x].append(eer_)
			#print("EER " + str(eer_))
			tprs[x].append(interp(mean_fpr, fpr, tpr))
			tprs[x][-1][0] = 0.0
			roc_auc = auc(fpr, tpr)
			aucs[x].append(roc_auc)
			plt.plot(fpr, tpr, lw=1, alpha=0.5, label='ROC fold %d (AUC = %0.2f)' % (nFoldNumber, roc_auc))


	# Confusion Matrix calculations
	if m_nClassCount != 2:
		y_pred = np.argmax(y_pred, axis=1)
		cm += confusion_matrix(Ytr[test].argmax(axis=1), y_pred)
	else:
		if bCategorical:
			# Commented code used when data in Y is [0, 1] and not [0]
			y_pred = np.argmax(y_pred, axis=1)
			cm += confusion_matrix(Ytr[test].argmax(axis=1), y_pred)
		else:
			# Commented code used when data in Y is one number and not [0, 1]
			y_pred = (y_pred > 0.5)
			y_pred = y_pred.astype(int)
			cm += confusion_matrix(Ytr[test], y_pred)	

	#arrMae.append(scores[2])
	try:
		del model # this is from global space
	except:
		pass
	AWSCTDClearSesion.reset_keras()
	
end = time.time()

tmExec = end - start
dTimeTrain = np.mean(arrTimeFit)
dTimeTest = np.mean(arrTimeTest)

dTimePredSum = np.sum(arrTimePredict)
dTimePredForOneSample = dTimePredSum / nAllSize
dTimeTest = np.mean(arrTimeTest)
print("All time            : %.7f" % tmExec)
print("Training time       : %.7f" % dTimeTrain)
print("Testing time        : %.7f" % dTimeTest)

print("Predicting time All : %.7f" % dTimePredSum)
print("Predicting time One : %.7f" % dTimePredForOneSample)

print(" Acc: %.2f%% (+/- %.2f)" % (np.mean(arrAcc), np.std(arrAcc)))

model = AWSCTDCreateModel.CreateModelImpl(m_sModel, m_nWordCount, m_nClassCount, m_nParametersCount, bCategorical)

sModel = str(model.to_json())
dAcc = np.mean(arrAcc)
dAccStd = np.std(arrAcc)
dLoss = np.mean(arrLoss)
dLossStd = np.std(arrLoss)

dAcc1 = arrAcc[0]
dAcc2 = arrAcc[1]
dAcc3 = arrAcc[2]
dAcc4 = arrAcc[3]
dAcc5 = arrAcc[4]

import sqlite3
con = sqlite3.connect('results.db')
sTestTag = m_sModel
result = (m_sDataFile, m_nParametersCount, m_nClassCount, nEpochs, nBatchSize, sModel, tmExec, dAcc, dLoss, dTimeTrain, dTimeTest, sTestTag, dAccStd, dLossStd, sTime, dTimePredForOneSample, dAcc1, dAcc2, dAcc3, dAcc4, dAcc5, sConfig)
sql = """INSERT INTO results 
         (File, ParamCount, ClassCount, Epochs, BatchSize, Model, Time, Acc, Loss, TimeTrain, TimeTest, Comment, AccStd, LossStd, ExecutionTime, PredictingOneTime, Acc1, Acc2, Acc3, Acc4, Acc5, Config)
         VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
      """
cur = con.cursor()
cur.execute(sql, result)
con.commit()

AWSCTDPlotAcc.plot_acc_loss(model_history, m_sModel, m_sDataFile, bCategorical, m_sWorkingDir)
AWSCTDPlotCM.plot_cm(cm, m_sModel, m_nClassCount, m_sDataFile, m_sWorkingDir)

# # Generate ROC figures
if bCategorical:
	sERR = ""
	arrClassNames = ("Benign", "Malware")
	import ntpath
	for x in range(m_nClassCount):
		plt.figure(x)
		#plt.plot([0, 1], [0, 1], linestyle='--', lw=1, color='r', label='Chance', alpha=.3)
		plt.plot([0, 1], [0, 1], linestyle='--', lw=1, color='r', alpha=.3)
		
		mean_tpr = np.mean(tprs[x], axis=0)
		mean_tpr[-1] = 1.0
		mean_auc = auc(mean_fpr, mean_tpr)
		std_auc = np.std(aucs[x])
		plt.plot(mean_fpr, mean_tpr, color='b', label=r'Mean ROC (AUC = %0.2f $\pm$ %0.2f)' % (mean_auc, std_auc), lw=2, alpha=.8)
		
		std_tpr = np.std(tprs[x], axis=0)
		tprs_upper = np.minimum(mean_tpr + std_tpr, 1)
		tprs_lower = np.maximum(mean_tpr - std_tpr, 0)
		plt.fill_between(mean_fpr, tprs_lower, tprs_upper, color='grey', alpha=.2,
						 label=r'$\pm$ 1 std. dev.')
		
		plt.xlim([-0.05, 1.05])
		plt.ylim([-0.05, 1.05])
		plt.xlabel('False Positive Rate')
		plt.ylabel('True Positive Rate')
		sTitle = 'ROC of ' + arrClassNames[x]
		plt.title(sTitle)
		plt.grid(True)
		plt.legend(loc="lower right")
		
		fileROC=m_sWorkingDir+'ROC/ROC_'+m_sModel+'_'
		fileROC+=ntpath.basename(m_sDataFile)+'_'
		fileROC+=arrClassNames[x]
		fileROC+='.svg'
		
		plt.savefig(fileROC, dpi=300, format='svg')
		
		mean_eer = np.mean(EER[x], axis=0)
		sERR += "[" + arrClassNames[x] + " " + str(mean_eer) + "]"
		
	print(sERR)
