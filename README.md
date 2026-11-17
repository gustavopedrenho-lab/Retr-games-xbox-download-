# Retro Games Launcher — Xbox 360

Protótipo homebrew para Xbox 360 RGH/JTAG, baseado no SDK aberto xsdk.

## O que esta versão 0.1 faz

- Abre em 720p e desenha uma tela de menu.
- Mostra seis plataformas: Arcade, Nintendo, Super NES, Mega Drive, Game Boy e Atari.
- Navega com D-pad, LB e RB.
- `A` tenta iniciar o emulador XEX configurado para a plataforma selecionada.
- `Y` mostra a ajuda.
- `BACK` fecha o launcher.

## Limitações honestas desta versão

- Ainda não lê a biblioteca automaticamente.
- Ainda não carrega capas gráficas.
- Não inclui ROMs, jogos ou emuladores.
- Os caminhos dos emuladores são exemplos e precisam ser ajustados em `src/main.c`.
- O emulador precisa ser um XEX compatível com o Xbox 360 e estar no caminho indicado.
- O código ainda precisa ser compilado e testado no console. Nenhum XEX pré-compilado acompanha este ZIP.

## Requisito

Use o repositório oficial do xsdk: https://github.com/xenosworks/xsdk

O xsdk documenta build em Debian (Linux) ou Debian no WSL2 do Windows 10/11. Winlator pode ser usado para editar/organizar os arquivos, mas não é um ambiente de build oficialmente documentado. O processo da toolchain usa cerca de 10 GB livres e constrói GCC/binutils/newlib para `powerpc-xsdk-elf`.

## Build oficial (Debian / WSL2)

1. Instale Debian 13 no WSL2 ou use Debian Linux.
2. Instale dependências:

```bash
sudo apt update
sudo apt install build-essential bison flex texinfo libgmp-dev libmpfr-dev libmpc-dev curl xz-utils patch python3 cmake ninja-build git
```

3. Baixe o SDK:

```bash
git clone https://github.com/xenosworks/xsdk.git ~/xsdk
```

4. Construa a toolchain:

```bash
cd ~/xsdk
PREFIX=$HOME/xsdk-gcc WORK=$HOME/xsdk-gcc-work JOBS=$(nproc) tools/toolchain/build.sh
```

5. Construa o SDK:

```bash
make -j$(nproc)
```

6. Copie esta pasta `RetroGamesLauncher` para o ambiente Debian e compile:

```bash
cmake -S RetroGamesLauncher -B RetroGamesLauncher/build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/xsdk/cmake/xsdk.cmake \
  -DXSDK_TOOLCHAIN=$HOME/xsdk-gcc \
  -DCMAKE_BUILD_TYPE=Release

cmake --build RetroGamesLauncher/build
```

O arquivo esperado será `RetroGamesLauncher/build/RetroGamesLauncher.xex`.

## Instalar no Xbox 360

1. Copie o XEX para uma pasta própria no HDD/USB usando FTP do Aurora ou outro método local.
2. Ajuste os caminhos dos emuladores em `src/main.c` e recompile.
3. Inicie o XEX pelo Aurora.
4. Teste primeiro sem apontar para conteúdo que não esteja instalado.

## Controles

- D-pad para cima/baixo ou esquerda/direita: muda a plataforma.
- LB/RB: plataforma anterior/próxima.
- A: inicia o XEX do emulador configurado.
- Y: ajuda.
- B: fecha a ajuda.
- BACK: sair.

## Próximas versões

- Leitura de configuração externa.
- Biblioteca de jogos por pasta.
- Capas e imagens.
- Favoritos e busca.
- Lançamento de ROM por launch data, apenas com emuladores que implementem esse protocolo.

## Créditos e licença

O projeto usa a API do xsdk, distribuído sob MIT. Consulte o repositório original e seus avisos de terceiros: https://github.com/xenosworks/xsdk
