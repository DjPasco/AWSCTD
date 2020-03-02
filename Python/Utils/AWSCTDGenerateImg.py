import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import numpy as np
import itertools

plt.rcParams['svg.fonttype'] = 'none'

# cm = np.matrix([[4.149e+03, 0.000e+00, 6.100e+01, 1.200e+02, 3.000e+00],
				# [3.000e+00, 0.000e+00, 2.000e+00, 1.000e+02, 0.000e+00],
				# [3.590e+02, 0.000e+00, 3.270e+02, 2.000e+01, 4.000e+00],
				# [1.590e+02, 0.000e+00, 2.600e+01, 1.565e+03, 5.000e+00],
				# [2.100e+01, 0.000e+00, 1.200e+01, 1.000e+00, 5.840e+02]])
				
def plot_confusion_matrix(cm, classes,
						  file='out_file.png',
						  cmap=plt.cm.Greens):
	plt.rcParams['xtick.bottom'] = plt.rcParams['xtick.labelbottom'] = False
	plt.rcParams['xtick.top'] = plt.rcParams['xtick.labeltop'] = True
		
	fig, ax = plt.subplots()
		
	np.set_printoptions(precision=0)
	#classesy = ['AdWare (a)', 'DangerousObject (b)', 'Downloader (c)', 'Trojan (d)', 'WebToolbar (e)']
	#classesy = ['a', 'b', 'c', 'd', 'e']
	#classesx = ['a', 'b', 'c', 'd', 'e']
	plt.imshow(cm, interpolation='nearest', cmap=cmap)
	#plt.title(title)
	#plt.colorbar()
	tick_marks = np.arange(len(classes))
	plt.xticks(tick_marks, classes)
	plt.yticks(tick_marks, classes)

	fmt = ".0f"# if normalize else "d"
	thresh = cm.max() / 2.
	for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
		plt.text(j, i, format(cm[i, j], fmt),
				 horizontalalignment="center",
				 color="white" if cm[i, j] > thresh else "black")

	plt.ylabel('True label')
	#ax.set_title('Predicted label')
	ax.set_xlabel('Predicted label')    
	ax.xaxis.set_label_position('top') 
	plt.tight_layout()

	plt.savefig(file, dpi=300, format='svg')

def plot_history(history, fileAcc, fileLoss):
	# Plot training & validation accuracy values
	plt.plot(history.history['acc'])
	#plt.plot(history.history['val_accuracy'])
	plt.title('Model accuracy')
	plt.ylabel('Accuracy')
	plt.xlabel('Epoch')
	plt.legend(['a', 'b', 'c', 'd', 'e'], loc='bottom right')
	plt.savefig(fileAcc, dpi=300, format='svg')

	# # Plot training & validation loss values
	# plt.plot(history.history['loss'])
	# #plt.plot(history.history['val_loss'])
	# plt.title('Model loss')
	# plt.ylabel('Loss')
	# plt.xlabel('Epoch')
	# plt.legend(['Train', 'Test'], loc='upper left')
	# plt.savefig(fileLoss, dpi=150)