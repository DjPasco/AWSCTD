import sys

if len(sys.argv) != 2 : 
	print("Parameters example: AWSCTD.py file_to_data.csv CNN")
	quit()

import os
import tensorflow as tf
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3' 

m_sDataFile = sys.argv[1]

import numpy as np
# fix random seed
np.random.seed(0)

import AWSCTDReadData

m_nParametersCount = 0
m_nClassCount = 0
m_nWordCount = 0
bCategorical = False
Xtr, Ytr, m_nParametersCount, m_nClassCount, m_nWordCount = AWSCTDReadData.ReadDataImpl(m_sDataFile, bCategorical)

import AWSCTDCreateModel

@property
def layers(self):
	# Historically, `sequential.layers` only returns layers that were added
	# via `add`, and omits the auto-generated `InputLayer`
	# that comes at the bottom of the stack.
	if self._layers and isinstance(self._layers[0], InputLayer):
		return self._layers[1:]
	return self._layers

from keras.utils import plot_model
import matplotlib.pyplot as plt
import ntpath

import os
m_sWorkingDir = os.getcwd()
m_sWorkingDir = m_sWorkingDir + '/'
print m_sWorkingDir

def RunModel(sModel):
	fileModel=m_sWorkingDir+'Model/MODEL_'+sModel+'_'
	fileModel+=ntpath.basename(m_sDataFile)
	fileModel+='.svg'

	model = AWSCTDCreateModel.CreateModelImpl(sModel, m_nWordCount, m_nClassCount, m_nParametersCount, bCategorical)
	plot_model(model, to_file=fileModel, show_shapes=False, show_layer_names=False)

RunModel("FCN")
RunModel("LSTM-FCN")
RunModel("GRU-FCN")
RunModel("AWSCTD-CNN-S")
RunModel("AWSCTD-CNN-LSTM")
RunModel("AWSCTD-CNN-GRU")
RunModel("AWSCTD-CNN-D")