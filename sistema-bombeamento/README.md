# Sistema de Bombeamento de Água
Este projeto é um acionador de um sistema de bombeamento de água.

## Descrição
O sistema é constituído por uma bomba, dois sensores de nível e duas caixas d'água, que se distinguem pela altura que estão instaladas, sendo que a inferior alimenta a superior. A lógica implementada prevê que quando a caixa superior está em 80% ou abaixo de sua capacidade a caixa inferior a abastece até atingir aproximadamente sua capacidade máxima - 95%. Porém existe uma condição que protege a bomba de operar sem água, que é a verificação se a caixa inferior está abastecida de pelo menos 20% de sua capacidade.

![Screenshot](https://github.com/hnovais95/pic/blob/master/sistema-bombeamento/plant.png)

## Materiais utilizados
* 1x PIC16F877A
* 2x Sensores de nível analógicos
* 1x Relé

## Implementação
O programa foi desenvolvido no software MPLAB X IDE com o compilador XC8. A implementação foi realizada no software PICSimLab, utilizando a placa PICGenios.

![Screenshot](https://github.com/hnovais95/pic/blob/master/sistema-bombeamento/screenshot_picsimlab.png)
