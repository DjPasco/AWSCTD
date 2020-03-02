import matplotlib
matplotlib.use('Agg')

import ntpath
import matplotlib.pyplot as plt

plt.rcParams['svg.fonttype'] = 'none'

def plot_acc_loss(model_history, sModel, sDataFile, bCategorical, sWorkingDir):	
	plt.figure("ACC")
	plt.title('Accuracy vs Epochs')
	plt.figure("LOSS")
	plt.title('Loss vs Epochs')
	nSize = len(model_history)
	sAccuracy = "acc"
	if bCategorical:
		sAccuracy = "categorical_accuracy"
	for x in range(nSize):
		sLabel  = 'Fold ' + str(x+1)
		
		plt.figure("ACC")
		plt.plot(model_history[x].history[sAccuracy],  label=sLabel)
		
		plt.figure("LOSS")
		plt.plot(model_history[x].history['loss'], label=sLabel)

	fileAcc=sWorkingDir+'ACCLOSS/ACC_'+sModel+'_'
	fileAcc+=ntpath.basename(sDataFile)
	fileAcc+='.svg'

	fileLoss=sWorkingDir+'ACCLOSS/LOSS_'+sModel+'_'
	fileLoss+=ntpath.basename(sDataFile)
	fileLoss+='.svg'

	plt.figure("ACC")
	plt.grid(True)
	plt.legend(loc="center right", fontsize = 'xx-large')
	plt.savefig(fileAcc, dpi=300, format='svg')

	plt.figure("LOSS")
	plt.grid(True)
	plt.legend(loc="center right", fontsize = 'xx-large')
	plt.savefig(fileLoss, dpi=300, format='svg')
	
