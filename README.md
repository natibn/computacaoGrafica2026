# computacaoGrafica2026

Repositório para a entrega das atividades da disciplina de Computação Gráfica do Curso de Ciência da Computação da Unisinos 2026/01

Tarefa 1 - Criando o ambiente de programação em Cenas 3D (Deadline 22/03/2026)

Tarefa 2 - Instanciando objetos na cena 3D (Deadline 05/05/2026) - Informações [aqui](Tarefa2/Tarefa2.md)

Atividade Vivencial 09/05/2026 - Informações encontram-se [aqui](Vivencial1/README_VIVENCIAL.md)

Tarefa 3 - Adicionando texturas (Deadline 19/05/2026) - Informações [aqui](Tarefa3/Tarefa3.md)

Tarefa 4 - Adicionando iluminação (Deadline 28/05/2026) e Atividade Vivencial 23/05/2026, conforme orientação do professor, foram entregues juntas.  - Informações [aqui](Tarefa4/Tarefa4.md)

Correção de texura do chimpanzé - Realizada em 25/05/2026 - Informações [aqui](CorrecaoTextura/CorrecaoTextura.md)

Tarefa 5 - Adicionando uma câmera em primeira pessoa - Informações [aqui](Tarefa5/Tarefa5.md)

Tarefa 6 - Definindo trajetórias para alguns objetos - Informações [aqui](Tarefa6/Tarefa6.md)

O programa `OBJSelection` carrega e inicializa os objetos, fontes de luz, posicionamento da câmera e parâmetros de frustum a partir de um arquivo de configuração no formato JSON:
[`CGCCHibrido/assets/Modelos3D/scene_config.json`](file:///c:/Users/natib/computacaoGrafica2026/CGCCHibrido/assets/Modelos3D/scene_config.json)

**Tutorial de Comandos — `OBJSelection`**

As seguintes teclas e controles estão disponíveis no programa `OBJSelection` (arquivo fonte: CGCCHibrido/src/OBJSelection.cpp).

- TAB : Seleciona o próximo objeto na cena.
- C : Alterna entre modo OBJETO (controle do objeto) e modo CÂMERA (primeira pessoa).
- W / S : Move o objeto (ou câmera) para frente / trás.
- A / D : Move o objeto (ou câmera) para esquerda / direita.
- Q / E : Move o objeto para cima / baixo.
- L / K : Aumenta / diminui a escala do objeto selecionado.
- X / Y / Z : Seleciona o eixo de rotação para aplicar giros.
- R / T : Gira o objeto no eixo selecionado (R = positivo, T = negativo).
- P : Alterna modo wireframe (exibe malha em linha).
- 1 / 2 / 3 : Alterna Key Light, Fill Light e Back Light respectivamente.
- ESC : Fecha a aplicação.

Controles de câmera (modo CÂMERA):
- Mouse movimento : altera yaw/pitch (olhar ao redor).
- Roda do mouse : altera o FOV (zoom).

Trajetórias paramétricas (funcionalidade adicionada):
- F : Adiciona a posição atual do objeto selecionado como ponto de controle da trajetória.
- G : Inicia / pausa a animação da trajetória (o objeto percorre os pontos ciclicamente).
- H : Limpa todos os pontos de controle da trajetória do objeto selecionado.
- O : Salva a trajetória atual do objeto em um arquivo chamado `traj_<nome>.txt` — agora salvo na pasta de assets detectada pelo programa (ex.: `CGCCHibrido/assets/`).
- I : Carrega a trajetória do arquivo `traj_<nome>.txt` presente na pasta de assets.
- + / = (ou teclado numérico +) : Aumenta a velocidade da animação da trajetória.
- - (ou teclado numérico -) : Diminui a velocidade da animação da trajetória.