#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <dirent.h>
#include <sstream>
#include <vector>

using namespace std;

// TrieNode para estrutura Trie
struct NoTrie {
    unordered_map<char, NoTrie*> filho;
    bool finalPalavra;
    unordered_map<string, vector<int>> ocorrencias; // Mapa para armazenar posicoes das palavras por arquivo (Hash)

    NoTrie() {
        finalPalavra = false;
    }
};

// Classe Trie
class Trie {
private:
    NoTrie* raiz;

public:
    Trie() {
        raiz = new NoTrie();
    }

    // Insere uma palavra na Trie
    void insere(const string& palavra, const string& nomeArquivo, int posicao) {
        NoTrie* atual = raiz;
        for (char c : palavra) {
            if (atual->filho.find(c) == atual->filho.end()) {
                atual->filho[c] = new NoTrie();
            }
            atual = atual->filho[c];
        }
        atual->finalPalavra = true;
        atual->ocorrencias[nomeArquivo].push_back(posicao); // Adiciona a posicao da palavra no arquivo
    }

    // Verifica se uma palavra esta na Trie
    bool busca(const string& palavra) {
        NoTrie* atual = raiz;
        for (char c : palavra) {
            if (atual->filho.find(c) == atual->filho.end()) {
                return false;
            }
            atual = atual->filho[c];
        }
        return atual != nullptr && atual->finalPalavra;
    }

    // Retorna as ocorrencias de uma palavra
    unordered_map<string, vector<int>> getOcorrencias(const string& palavra) {
        NoTrie* atual = raiz;
        for (char c : palavra) {
            if (atual->filho.find(c) == atual->filho.end()) {
                return {}; // Retorna um mapa vazio se a palavra nao existir na Trie
            }
            atual = atual->filho[c];
        }
        return atual->ocorrencias;
    }

    // Funcao para encontrar arquivos que contenham todas as palavras de uma lista e calcular proximidade
    vector<pair<string, double>> achaArquivosComProximidade(const vector<string>& frase) {
        unordered_map<string, vector<int>> arquivoParaContagemPalavras;
        unordered_map<string, double> arquivoParaProximidade;

        for (const auto& palavra : frase) {
            auto ocorrencias = getOcorrencias(palavra);
            for (const auto& par : ocorrencias) {
                arquivoParaContagemPalavras[par.first].push_back(palavra.size());

                // Calcular a menor distancia entre as palavras
                if (arquivoParaProximidade.find(par.first) == arquivoParaProximidade.end()) {
                    arquivoParaProximidade[par.first] = DBL_MAX;
                }

                for (const auto& outraPalavra : frase) {
                    if (palavra != outraPalavra) {
                        auto outrasOcorrencias = getOcorrencias(outraPalavra);
                        for (const auto& outroPar : outrasOcorrencias) {
                            if (outroPar.first == par.first) {
                                for (int pos : par.second) {
                                    for (int outraPos : outroPar.second) {
                                        double distancia = abs(pos - outraPos);
                                        if (distancia < arquivoParaProximidade[par.first]) {
                                            arquivoParaProximidade[par.first] = distancia;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        vector<pair<string, double>> resultado;
        for (const auto& par : arquivoParaContagemPalavras) {
            if (par.second.size() == frase.size()) {
                resultado.push_back({ par.first, arquivoParaProximidade[par.first] });
            }
        }

        sort(resultado.begin(), resultado.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
            return a.second < b.second;
            });

        return resultado;
    }

    // Libera a memoria alocada pela Trie
    void limpar(NoTrie* no) {
        if (no == nullptr) return;
        for (auto& it : no->filho) {
            limpar(it.second);
        }
        delete no;
    }

    ~Trie() {
        limpar(raiz);
    }
};

// Funcao auxiliar para verificar se o arquivo aberto e do formato .pdf
bool extensao(const string& stringCompleta, const string & final) {
    if (stringCompleta.length() >= final.length()) {
        return (stringCompleta.compare(stringCompleta.length() - final.length(), final.length(), final) == 0);
    }
    else {
        return false;
    }
}

// Funcao para remover acentos e caracteres especiais
string normalizar(const string& str) {
    string normalizado;
    for (char c : str) {
        if (c >= 'A' && c <= 'Z') {
            normalizado += c - 'A' + 'a'; // Converte para minuscula
        }
        else if (c >= 'a' && c <= 'z') {
            normalizado += c; // Mantem letras minusculas
        }
        else if (c >= '0' && c <= '9') {
            normalizado += c; // Mantem numeros
        }
        // Acentos e caracteres especiais
        else if (c == '�' || c == '�' || c == '�' || c == '�') normalizado += 'a';
        else if (c == '�' || c == '�') normalizado += 'e';
        else if (c == '�') normalizado += 'i';
        else if (c == '�' || c == '�' || c == '�') normalizado += 'o';
        else if (c == '�') normalizado += 'u';
        else if (c == '�') normalizado += 'c';
        // Espaco em branco
        else if (isspace(c)) normalizado += ' ';
    }
    return normalizado;
}

// Funcao para converter um arquivo PDF em texto (.txt)
string pdfParaString(const string& arquivoPdf) {
    string arquivoTxt;
    try {
        // Abrir o documento PDF
        auto documento = unique_ptr<poppler::document>(poppler::document::load_from_file(arquivoPdf));

        if (!documento) {
            cerr << "Erro ao abrir o arquivo PDF: " << arquivoPdf << endl;
            return " ";
        }

        string texto = "";
        // Extrair texto de cada pagina
        for (int i = 0; i < documento->pages(); ++i) {
            auto pagina = unique_ptr<poppler::page>(documento->create_page(i));
            if (!pagina) {
                cerr << "Erro ao criar a pagina " << i + 1 << " do arquivo PDF: " << arquivoPdf << endl;
                continue;
            }
            texto += pagina->text().to_latin1();
        }

        // Escrever texto extraido para um arquivo .txt com o mesmo nome
        arquivoTxt = arquivoPdf.substr(0, arquivoPdf.find_last_of(".")) + ".txt";
        ofstream output(arquivoTxt);
        output << texto;
        output.close();

        cout << "Texto extraido de " << arquivoPdf << " e salvo em " << arquivoTxt << endl;
    }
    catch (const exception& e) {
        cerr << "Erro ao processar o arquivo " << arquivoPdf << ": " << e.what() << endl;
    }
    return arquivoTxt;
}

// Funcao para listar arquivos PDF em um diretorio
vector<string> listaArquivos(const string& diretorio) {
    vector<string> resultados;
    DIR* dir;
    struct dirent* ent;

    // Abrir o diretorio especificado
    if ((dir = opendir(diretorio.c_str())) != NULL) {
        // Percorrer todos os arquivos dentro do diretorio
        while ((ent = readdir(dir)) != NULL) {
            // Ignorar "." e ".."
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            // Verificar se o arquivo e um PDF (pode ser ajustado conforme necessario)
            if (extensao(ent->d_name, ".pdf")) {
                string path = diretorio + "/" + ent->d_name;
                // Chamar a funcao para converter PDF para texto
                string texto = pdfParaString(path);
                resultados.push_back(texto);
            }
        }
        closedir(dir);
    }
    else {
        // Se nao conseguir abrir o diretorio, exibir um erro
        cerr << "Erro: Nao foi possivel abrir o diretorio " << diretorio << endl;
    }

    return resultados;
}

int main() {
    string diretorio;
    cout << "Digite o caminho para o diretorio onde estao os arquivos PDF: ";
    getline(cin, diretorio);
    vector<string> arquivosTxt = listaArquivos(diretorio);

    Trie trie;

    // Processar cada arquivo .txt gerado e inserir as palavras na Trie
    for (const auto& arquivoTxt : arquivosTxt) {
        ifstream inputFile(arquivoTxt);
        if (inputFile.is_open()) {
            string palavra;
            int posicao = 0;
            while (inputFile >> palavra) {
                // Converter todas as palavras para letras minusculas (opcional)
                transform(palavra.begin(), palavra.end(), palavra.begin(), ::tolower);
                // Normalizar a palavra (remover acentos e pontuacao)
                palavra = normalizar(palavra);
                // Inserir na Trie, passando o nome do arquivo e a posicao da palavra
                trie.insere(palavra, arquivoTxt, posicao);
                posicao++;
            }
            inputFile.close();
        }
        else {
            cerr << "Erro ao abrir o arquivo " << arquivoTxt << endl;
        }
    }

    while (true) {
        int opcao;
        cout << "\nEscolha uma opcao de busca:\n1. Buscar uma palavra\n2. Buscar varias palavras\n0. Sair\nEscolha: ";
        cin >> opcao;
        cin.ignore(); // Limpar o buffer do cin
        string entrada;
        switch (opcao) {

        case 0:
            return 0;


        case 1: {
            cout << "Digite uma palavra para buscar: ";
            cin >> entrada;
            cin.ignore(); // Limpar o buffer do cin
            transform(entrada.begin(), entrada.end(), entrada.begin(), ::tolower);
            entrada = normalizar(entrada);

            if (trie.busca(entrada)) {
                unordered_map<string, vector<int>> ocorrencias = trie.getOcorrencias(entrada);

                if (ocorrencias.empty()) {
                    cout << "A palavra '" << entrada << "' nao foi encontrada nos arquivos." << endl;
                }
                else {
                    vector<pair<string, int>> ocorrenciasOrganizadas;
                    for (const auto& par : ocorrencias) {
                        ocorrenciasOrganizadas.push_back({ par.first, par.second.size() });
                    }

                    // Ordenar por numero de ocorrencias (decrescente)
                    sort(ocorrenciasOrganizadas.begin(), ocorrenciasOrganizadas.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
                        return b.second < a.second;
                        });

                    cout << "A palavra '" << entrada << "' foi encontrada nos seguintes arquivos:" << endl;
                    for (const auto& par : ocorrenciasOrganizadas) {
                        cout << "- " << par.first << ": " << par.second << " ocorrencias" << endl;
                    }
                }
            }
            else {
                cout << "A palavra '" << entrada << "' nao foi encontrada nos arquivos." << endl;
            }
            break;
        }
        case 2: {
            cout << "Digite as palavras para buscar, separadas por espaco: ";
            getline(cin, entrada);
            transform(entrada.begin(), entrada.end(), entrada.begin(), ::tolower);
            entrada = normalizar(entrada);

            vector<string> palavras;
            stringstream ss(entrada);
            string palavra;
            while (ss >> palavra) {
                palavras.push_back(palavra);
            }

            vector<pair<string, double>> arquivos = trie.achaArquivosComProximidade(palavras);

            if (arquivos.empty()) {
                cout << "Nenhum arquivo contem todas as palavras fornecidas." << endl;
            }
            else {
                cout << "Arquivos que contem todas as palavras fornecidas, ordenados por proximidade das palavras:" << endl;
                for (const auto& arquivo : arquivos) {
                    cout << "- " << arquivo.first << ": proximidade " << arquivo.second << endl;
                }
            }
            break;
        }
        default: {
            cout << "Opcao invalida. Por favor, tente novamente." << endl;
        }
      }
    }

    return 0;
}