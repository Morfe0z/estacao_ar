// Copie este arquivo para "secrets.h" (na mesma pasta) e preencha.
// O secrets.h fica so no seu computador: ele esta no .gitignore.
#pragma once

// Rede propria da estacao (sempre ligada). Pagina em http://192.168.4.1
const char* AP_NOME  = "EstacaoAr";
const char* AP_SENHA = "troque-esta-senha";   // minimo 8 caracteres

// Rede com internet para enviar ao ThingSpeak (deixe "" para nao usar)
const char* WIFI_NOME  = "";
const char* WIFI_SENHA = "";

// ThingSpeak: "Write API Key" do canal (deixe "" para nao enviar)
const char* THINGSPEAK_CHAVE = "";
