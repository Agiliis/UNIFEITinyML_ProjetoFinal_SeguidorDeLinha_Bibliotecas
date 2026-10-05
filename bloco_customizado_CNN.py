import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, InputLayer, Lambda, Normalization, Dropout
from tensorflow.keras.callbacks import EarlyStopping

PWM_MAX = 100.0

# 1. Ajustar o dataset original
def split_features_and_targets(features, dummy_label):
    y = (features[-2:] + 60)  / (PWM_MAX + 60)
    return features, y 

train_dataset = train_dataset.map(split_features_and_targets)
validation_dataset = validation_dataset.map(split_features_and_targets)

BATCH_SIZE = 32
train_dataset = train_dataset.batch(BATCH_SIZE, drop_remainder=False)
validation_dataset = validation_dataset.batch(BATCH_SIZE, drop_remainder=False)

# 2. Criar e treinar a camada de Normalização
# O método adapt() precisa ler os dados brutos de treino para calcular médias e desvios.
# Mapeamos o dataset de treino para isolar apenas as 6 features reais.
features_only_dataset = train_dataset.map(lambda x, y: x[:, :-2])

norm_layer = Normalization(axis=-1)
norm_layer.adapt(features_only_dataset) # A camada "aprende" os limites estatísticos dos sensores

# 3. Construir o Modelo com a Normalização Embutida
# 1. Arquitetura mais robusta contra Overfitting
model = Sequential([
    InputLayer(input_shape=(input_length, )), 
    Lambda(lambda x: x[:, :-2]),
    norm_layer,
    
    # Reduzimos levemente a capacidade e adicionamos Dropout
    Dense(64, activation='relu'),
    Dropout(0.25), # Desliga 25% dos neurônios aleatoriamente no treino
    
    Dense(32, activation='relu'),
    Dropout(0.1),  # Desliga 10%
    
    Dense(16, activation='relu'),
    
    Dense(2, activation='sigmoid')
])

model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=0.001), 
    loss=tf.keras.losses.Huber(), 
    metrics=['mae']
)

# 2. Configurar a Parada Antecipada
# Se o val_loss não melhorar por 15 épocas seguidas, o treino para.
early_stop = EarlyStopping(
    monitor='val_loss', 
    patience=15, 
    restore_best_weights=True
)

# Garantir que o early_stop não sobrescreva os callbacks obrigatórios do Edge Impulse
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
