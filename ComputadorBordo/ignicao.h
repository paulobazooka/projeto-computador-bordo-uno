// attachInterrupt(digitalPinToInterrupt(2),changeScreen,LOW);



 // Função que muda de tela de acordo com o estado do pino 2. Se low, chama a função.
void changeScreen(){  
 menu++;
 if(menu>5) menu=0;
 change = true;
 // delay(250);
}
