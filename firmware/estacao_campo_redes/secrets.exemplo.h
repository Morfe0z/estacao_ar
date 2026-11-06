// Copie este arquivo para "secrets.h" (na mesma pasta) e preencha.
// O secrets.h fica so no seu computador: ele esta no .gitignore.
#pragma once

// Rede propria da estacao (sempre ligada). Pagina em http://192.168.4.1
const char* AP_NOME  = "EstacaoAr";
const char* AP_SENHA = "troque-esta-senha";   // minimo 8 caracteres

// Redes com internet para enviar ao ThingSpeak.
// A estacao procura as redes desta lista e entra na que tiver melhor sinal.
// Rede comum: { "nome", "senha", "" }
// Rede com login (eduroam e parecidas): { "nome", "senha", "login" }
const RedeWifi REDES[] = {
  { "NOME_DA_REDE_DE_CASA", "SENHA",  "" },
  { "NOME_DA_REDE_DO_LAB",  "SENHA",  "" },
  { "eduroam",              "SENHA",  "LOGIN_INSTITUCIONAL" },
};

// ThingSpeak: "Write API Key" do canal (deixe "" para nao enviar)
const char* THINGSPEAK_CHAVE = "";
// ThingSpeak: numero do canal ("Channel ID", no topo da pagina do canal)
const char* THINGSPEAK_CANAL = "";
