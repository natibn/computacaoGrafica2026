Para esta tarefa, alterei o arquivo [OBJSelection.cpp](../CGCCHibrido/src/OBJSelection.cpp). Abaixo explico como testar.

- `TAB`: selecionar próximo objeto
- `F`: adicionar a posição atual do objeto como ponto de controle
- `G`: iniciar / pausar animação da trajetória
- `H`: limpar pontos de controle da trajetória
- `O`: salvar trajetória em arquivo (`traj_<nome>.txt`) — agora salvo em `assets/Modelos3D` (ou na pasta `assets` detectada)
- `I`: carregar trajetória a partir do arquivo correspondente em `assets`
- `+` / `=` ou `Teclado Numérico +`: aumentar velocidade da trajetória
- `-` ou `Teclado Numérico -`: diminuir velocidade da trajetória

- Os arquivos `traj_*.txt` são gravados no diretório de assets detectado automaticamente pelo executável — normalmente `CGCCHibrido/assets/Modelos3D` ou `CGCCHibrido/assets` dependendo da estrutura.

Aqui tem dois exemplos de trajetória de objetos: [traj_Cube.txt](../CGCCHibrido/assets/Modelos3D/traj_Cube.txt) e [traj_Suzanne.txt](../CGCCHibrido/assets/Modelos3D/traj_Suzanne.txt)