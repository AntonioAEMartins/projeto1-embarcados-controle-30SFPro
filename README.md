# projeto1-embarcados-controle-30SFPro
## Grupo: Antônio Amaral e Arthur Barreto

### SF30Pro

#### O que será feito no Projeto:

No projeto 1 de Embarcados devemos abordar o uso de diversos sensores (analogicos e digitais) a partir da criação de filas de esperaça e gerenciamento de processador, utilizando o ATMEL para processar todos os botoões, sensores e algoritmos e transmiti-los ao computador que reconhecerá os inputs e outputs conectando o controle a interface do Jogo.

Com isso em mente, o grupo buscou procurar um controle que poderia ser utilizado de báse, sendo ele o SF30 Pro, controle produzido pela empresa 8bitdo (https://www.8bitdo.com/sn30pro-sf30pro/) para serem utilizados em jogos 8 bits. O objetivo é pegar a estrutura física do controle (Frame de Plástico), produzir ele nos laborátorios do Insper, e utilizar 8 botões e 2 joysticks para controlar o jogo.

Com a finalidade de implementar outros recursos também será testado utilizar dois Buzzers para reproduzir alguns efeitos especiais dos jogos, como barulhos de tiros, pulos... - que podem ser reproduzidos através da transferencia do mesmo para a Placa, ou até mesmo da programação das frequências e tempos no ATMEL. Outra implementação que será feita é a adição de um sensor para medir a inclinação do controle, que será acionada em jogos de corrida possibilitando o controle de ser ao mesmo tempo um volante e controle analogico/digital. Por fim tudo isso deverá ser feito via Bluetooth.

#### Quais peças serão utilizadas:

1. 8xBotões
2. 2xJoysticks
3. 3xLeds (branco, verde e amarelo)
4. 1xATMEL Same70Xplained
5. 2xProtoBoard
6. Jumpers
7. 1x HW-123
8. 1xReceptor Bluetooth / micro-controlador
9. 2xBuzzers

#### Divisão de Objetivos:

1. Implementação de comunicação Serial (cabo)
2. Implementação de LEDS 
3. Implementação de Botões (Nível de código)
4. Implementação de JoySticks (Nível de código)
5. Funcionalidades de Botões (devem funcionar em jogos)
6. Funcionalidades de JoySticks (devem funcionar em jogos)

Após estas 6 etapas, o projeto terá um estágio de organização de código, para possibilitar trabalhar com todos os botões, leds e joysticks de forma paralela com pequeno tempo de resposta. Após isso, será impelementado o Receptor Bluetooth, em conjunto com outras funcionalidades.
