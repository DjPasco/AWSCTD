from keras.layers import CuDNNGRU, CuDNNLSTM, SimpleRNN, Convolution1D, Conv1D, Conv2D, Embedding, GlobalMaxPooling1D
from keras.models import Sequential
from keras.layers import Dense
from keras.layers import Dropout
from keras.layers import GlobalMaxPooling2D
from keras.layers import GlobalAveragePooling1D
from keras.layers import AveragePooling1D
from keras.layers import AveragePooling2D
from keras.layers import MaxPooling2D
from keras.layers import MaxPooling1D
from keras.layers import Flatten
from keras.layers import LSTM
from keras.layers import TimeDistributed
from keras.layers import Reshape
from keras.layers.normalization import BatchNormalization
from keras import optimizers
from keras.applications.inception_v3 import InceptionV3
from keras.layers import Input
from keras import backend as K
from keras.layers.convolutional_recurrent import ConvLSTM2D
from keras.layers import Concatenate
from keras.layers import Add
from keras.models import Model
from keras.layers import ReLU
from keras.layers.merge import concatenate	
	
def AddLastDenseLayer(merged, bCategorical, nClassCount):
	outputs = Dense(nClassCount if bCategorical else 1, activation='softmax' if bCategorical else 'sigmoid')(merged)
	return outputs
	
def CreateNewGRU(nWordCount, nClassCount, nParametersCount, bCategorical):
	print "GRU-FCN"
	inputs = Input(shape=(nParametersCount, nWordCount))
	conv1 = Conv1D(filters=128, kernel_size=8, padding='same',activation='tanh', kernel_initializer='he_uniform')(inputs)
	BN1 = BatchNormalization(epsilon=0.01)(conv1)
	relu1 = ReLU()(BN1)
	conv2 = Conv1D(filters=256, kernel_size=5, padding='same',activation='tanh', kernel_initializer='he_uniform')(relu1)
	BN2 = BatchNormalization(epsilon=0.01)(conv2)
	relu2 = ReLU()(BN2)
	conv3 = Conv1D(filters=128, kernel_size=3, padding='same',activation='tanh', kernel_initializer='he_uniform')(relu2)
	BN3 = BatchNormalization(epsilon=0.01)(conv3)
	relu3 = ReLU()(BN3)
	pool1 = GlobalAveragePooling1D()(relu3)
	gru2 = CuDNNGRU(units=8)(inputs)
	dropout = Dropout(0.8)(gru2)
	merged = concatenate([pool1, dropout])
	outputs = AddLastDenseLayer(merged, bCategorical, nClassCount)
	model = Model(inputs=[inputs], outputs=outputs)
	
	return model
	
def CreateNew(nWordCount, nClassCount, nParametersCount, bCategorical):
	print "FCN"
	inputs = Input(shape=(nParametersCount, nWordCount))
	conv1 = Conv1D(filters=128, kernel_size=8, padding='same',activation='tanh')(inputs)
	BN1 = BatchNormalization(epsilon=0.01)(conv1)
	relu1 = ReLU()(BN1)
	conv2 = Conv1D(filters=256, kernel_size=5, padding='same',activation='tanh')(relu1)
	BN2 = BatchNormalization(epsilon=0.01)(conv2)
	relu2 = ReLU()(BN2)
	conv3 = Conv1D(filters=128, kernel_size=3, padding='same',activation='tanh')(relu2)
	BN3 = BatchNormalization(epsilon=0.01)(conv3)
	relu3 = ReLU()(BN3)
	pool = GlobalAveragePooling1D()(relu3)
	outputs = AddLastDenseLayer(pool, bCategorical, nClassCount)
	model = Model(inputs=[inputs], outputs=outputs)
	
	return model
	
def CreateNewLSTM(nWordCount, nClassCount, nParametersCount, bCategorical):
	print "LSTM-FCN"
	inputs = Input(shape=(nParametersCount, nWordCount))
	conv1 = Conv1D(filters=128, kernel_size=8, padding='same',activation='tanh')(inputs)
	BN1 = BatchNormalization(epsilon=0.01)(conv1)
	relu1 = ReLU()(BN1)
	conv2 = Conv1D(filters=256, kernel_size=5, padding='same',activation='tanh')(relu1)
	BN2 = BatchNormalization(epsilon=0.01)(conv2)
	relu2 = ReLU()(BN2)
	conv3 = Conv1D(filters=128, kernel_size=3, padding='same',activation='tanh')(relu2)
	BN3 = BatchNormalization(epsilon=0.01)(conv3)
	relu3 = ReLU()(BN3)
	pool1 = GlobalAveragePooling1D()(relu3)
	lstm3 = CuDNNLSTM(units=nParametersCount)(inputs)
	dropout = Dropout(0.8)(lstm3)
	merged = concatenate([pool1, dropout])
	outputs = AddLastDenseLayer(merged, bCategorical, nClassCount)
	model = Model(inputs=[inputs], outputs=outputs)
	
	return model
	
def CreateOLDGRU(nWordCount, nClassCount, nParametersCount, bCategorical):
	print "AWSCTD-CNN-GRU"
	nSlidingWindow = 6
	inputs = Input(shape=(nParametersCount, nWordCount))
	conv1OLD = Conv1D(filters=nParametersCount, kernel_size=nSlidingWindow, padding='same',activation='tanh')(inputs)
	poolOLD  = GlobalMaxPooling1D()(conv1OLD)
	gru2 = CuDNNGRU(units=8)(inputs)
	dropout = Dropout(0.8)(gru2)
	merged = concatenate([poolOLD, dropout])
	outputs = AddLastDenseLayer(merged, bCategorical, nClassCount)
	model = Model(inputs=[inputs], outputs=outputs)
	
	return model
	
def CreateOLDLSTM(nWordCount, nClassCount, nParametersCount, bCategorical):
	print "AWSCTD-CNN-LSTM"
	nSlidingWindow = 6
	inputs = Input(shape=(nParametersCount, nWordCount))
	conv1OLD = Conv1D(filters=nParametersCount, kernel_size=nSlidingWindow, padding='same',activation='tanh')(inputs)
	poolOLD  = GlobalMaxPooling1D()(conv1OLD)
	lstm3 = CuDNNLSTM(units=nParametersCount)(inputs)
	dropout = Dropout(0.8)(lstm3)
	merged = concatenate([poolOLD, dropout])
	outputs = AddLastDenseLayer(merged, bCategorical, nClassCount)
	
	model = Model(inputs=[inputs], outputs=outputs)
	
	return model
	
def CreateCNN(nWordCount, nClassCount, nParametersCount, bCategorical):
	print "AWSCTD-CNN-D"
	nSlidingWindow = 6
	inputs = Input(shape=(nParametersCount, nWordCount))
	CNN = Conv1D(filters=nParametersCount, kernel_size=nSlidingWindow, padding='same',activation='tanh')(inputs)
	poolOLD  = GlobalMaxPooling1D()(CNN)
	outputs = AddLastDenseLayer(poolOLD, bCategorical, nClassCount)
	model = Model(inputs=[inputs], outputs=outputs)
	
	return model
	
def CreateCNNS(nWordCount, nClassCount, nParametersCount, bCategorical):
	print "AWSCTD-CNN-S"
	nSlidingWindow = 6
	inputs = Input(shape=(nParametersCount, nWordCount))
	CNN = Conv1D(filters=256, kernel_size=nSlidingWindow, padding='same',activation='tanh')(inputs)
	poolOLD  = GlobalMaxPooling1D()(CNN)
	outputs = AddLastDenseLayer(poolOLD, bCategorical, nClassCount)
	model = Model(inputs=[inputs], outputs=outputs)
	
	return model
	
def CreateModelImpl(sModel, nWordCount, nClassCount, nParametersCount, bCategorical):
	print "nWordCount: "+ str(nWordCount)
	print "nClassCount: "+ str(nClassCount)
	print "nParametersCount: "+ str(nParametersCount)
	model = Sequential()
	if sModel == "AWSCTD-CNN-LSTM":
		model = CreateOLDLSTM(nWordCount, nClassCount, nParametersCount, bCategorical)
	elif sModel == "AWSCTD-CNN-GRU":
		model = CreateOLDGRU(nWordCount, nClassCount, nParametersCount, bCategorical)
	elif sModel == "FCN":
		model = CreateNew(nWordCount, nClassCount, nParametersCount, bCategorical)
	elif sModel == "GRU-FCN":
		model = CreateNewGRU(nWordCount, nClassCount, nParametersCount, bCategorical)
	elif sModel == "LSTM-FCN":
		model = CreateNewLSTM(nWordCount, nClassCount, nParametersCount, bCategorical)
	elif sModel == "AWSCTD-CNN-D":
		model = CreateCNN(nWordCount, nClassCount, nParametersCount, bCategorical)
	elif sModel == "AWSCTD-CNN-S":
		model = CreateCNNS(nWordCount, nClassCount, nParametersCount, bCategorical)

	if bCategorical:
		model.compile(loss='categorical_crossentropy', optimizer="Adam", metrics=['categorical_accuracy'])
	else:
		model.compile(loss='binary_crossentropy', optimizer="Adam", metrics=['accuracy']) 
		
	return model