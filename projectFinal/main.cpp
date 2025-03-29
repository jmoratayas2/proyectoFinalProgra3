#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;
const string API_KEY = "AIzaSyDjUcy9IZ8lnzbpRnltVrU7rS4_bNqunUc"; //variable global, es el api key para acceder al api de google.

// Estructura para verificar el lenguaje
struct Lenguajes {
    string lenguaje;
    string nombre;
};

//Insertamos el listado de idiomas
Lenguajes lenguajes[] = {
    {"it", "Italiano"},
    {"en", "Ingles"},
    {"fr", "Frances"},
    {"de", "Aleman"}
};

// Función para manejar la respuesta de la API
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string translateText(const string& text, const string& targetLanguage) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        string url = "https://translation.googleapis.com/language/translate/v2?key=" + API_KEY;

        string jsonBody = "{\"q\": \"" + text + "\", \"target\": \"" + targetLanguage + "\"}";

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLoption::CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLoption::CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLoption::CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLoption::CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLoption::CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLoption::CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {//Verificamos la respuesta del api
            cerr << "Error en la solicitud: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return readBuffer;
}

string extractTranslation(const string& jsonResponse) {
    json responseJson = json::parse(jsonResponse);
    return responseJson["data"]["translations"][0]["translatedText"];
}

int main() {
    string text;
    cout<<"Ingresa el texto en espanol a traducir"<<endl;
    cin>>text;

    for (const auto& idioma : lenguajes) {
        string response = translateText(text, idioma.lenguaje);
        string translatedText = extractTranslation(response);

        if (!translatedText.empty()) {
            cout << "Texto traducido al "<<idioma.nombre <<": "<< translatedText << endl;
        } else {
            cerr << "Error al procesar la respuesta JSON." << endl;
        }
    }
    return 0;
}
