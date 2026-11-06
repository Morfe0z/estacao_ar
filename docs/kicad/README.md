# Placa final no KiCad

Guia passo a passo para desenhar no KiCad a placa principal da estação (ESP32 encaixado + resistores + bornes dos sensores) e fabricá-la.

## Decisões
- **Programa:** KiCad 10 (AppImage 10.0.8 no Linux do laboratório). Use a mesma versão em todos os computadores: a 9 não abre arquivos salvos na 10.
- **Fabricação:** placa caseira na universidade, por **transferência térmica de toner** (impressão a laser ou cópia xerox, prensa térmica, corrosão em percloreto de ferro).
- **Regras de desenho para placa caseira:**
  - face simples: cobre só embaixo (camada `B.Cu`), componentes por cima;
  - trilhas de sinal com 1 mm; 5 V e GND com 1,5 a 2 mm;
  - pelo menos 0,5 mm de isolação entre cobres;
  - ilhas de cerca de 2,2 mm, com furos de 0,8 a 1,0 mm (furação manual).
- Nomes de arquivos e do projeto **sem acento nem ç** (`placa_estacao`).

## Etapas
| Etapa | Conteúdo | Situação |
|---|---|---|
| [K1](k1-instalar-e-criar-projeto.md) | Instalar o KiCad e criar o projeto | ✅ concluída (2026-11-06) |
| [K2a](k2a-esquematico-colocar-pecas.md) | Esquemático: colocar as peças | 🛠️ em andamento |
| K2b | Esquemático: ligações, com a ordem real dos pinos do ESP32 | ⏳ |
| K2c | Verificação elétrica automática (ERC) | ⏳ |
| K3 | Footprints de cada peça | ⏳ |
| K4 | Layout da placa e verificação de regras (DRC) | ⏳ |
| K5 | Impressão 1:1 para transferência, corrosão, furação e conferência | ⏳ |

Os arquivos do projeto KiCad entram em `hardware/kicad/` quando o esquemático estiver pronto.
