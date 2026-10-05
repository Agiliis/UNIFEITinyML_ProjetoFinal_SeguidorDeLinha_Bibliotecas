import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, InputLayer, Lambda, Normalization, Dropout, Conv1D, Flatten, Reshape, MaxPooling1D
from tensorflow.keras.callbacks import EarlyStopping

PWM_MAX = 100.0

# 1. Nova Normalização: Mapeia de -60 a 100 para a escala 0.0 a 1.0
def split_features_and_targets(features, dummy_label):
    y = (features[-2:] + 60.0) / (PWM_MAX + 60.0)
    return features, y 

train_dataset = train_dataset.map(split_features_and_targets)
validation_dataset = validation_dataset.map(split_features_and_targets)

BATCH_SIZE = 32
train_dataset = train_dataset.batch(BATCH_SIZE, drop_remainder=False)
validation_dataset = validation_dataset.batch(BATCH_SIZE, drop_remainder=False)

# 2. Normalização Z-Score isolando apenas os 6 sensores brutos
features_only_dataset = train_dataset.map(lambda x, y: x[:, :-2])
norm_layer = Normalization(axis=-1)
norm_layer.adapt(features_only_dataset) 

# 3. Arquitetura CNN 1D para 6 Sensores sem janela temporal
model = Sequential([
    InputLayer(input_shape=(input_length, )), 
    
    # Corta os 2 alvos no final do array, sobrando os 6 sensores
    Lambda(lambda x: x[:, :-2]),
    norm_layer,
    
    # Transforma o array tabular numa sequência 1D para a convolução
    # Eixo X = 6 Sensores enfileirados, Eixo Y = 1 (Sem profundidade de memória)
    Reshape((6, 1)),
    
    # Desliza um filtro por blocos de 3 sensores adjacentes para entender o gradiente da linha
    Conv1D(filters=32, kernel_size=3, activation='relu', padding='same'),
    
    # Resume a informação espacial (reduz a dimensão pela metade focando nos sinais mais fortes)
    MaxPooling1D(pool_size=2),
    
    Conv1D(filters=16, kernel_size=2, activation='relu', padding='same'),
    
    Flatten(),
    Dense(32, activation='relu'),
    Dropout(0.1),
    
    # Saída Sigmoid garante que a previsão matemática nunca fuja de 0.0 a 1.0
    Dense(2, activation='sigmoid')
])

model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=0.001), 
    loss=tf.keras.losses.Huber(), 
    metrics=['mae']
)

# 4. Parada Antecipada
early_stop = EarlyStopping(
    monitor='val_loss', 
    patience=15, 
    restore_best_weights=True
)

if type(callbacks) == list:
    callbacks.append(early_stop)
else:
    callbacks = [callbacks, early_stop]

EPOCHS = 150 

model.fit(
    train_dataset, 
    epochs=EPOCHS, 
    validation_data=validation_dataset, 
    verbose=2,
    callbacks=callbacks
)
