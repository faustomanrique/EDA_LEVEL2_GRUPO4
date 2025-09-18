/**
 * @brief Lequel? language identification based on trigrams
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 *
 * @cite https://towardsdatascience.com/understanding-cosine-similarity-and-its-application-fd42f585296a
 */

#include <cmath>
#include <codecvt>
#include <locale>
#include <iostream>

#include "Lequel.h"

using namespace std;

/**
 * @brief Builds a trigram profile from a given text.
 *
 * @param text Vector of lines (Text)
 * @return TrigramProfile The trigram profile
 */
TrigramProfile buildTrigramProfile(const Text& text)
{
    wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    TrigramProfile trigProfReturn;
    
    for (std::string line : text)
    {
        if (!line.empty() && line.back() == '\r')           //borrar el CRLF que aparece en algunos archivos
            line.pop_back();

        if (line.empty())                                   //Si la linea es null entonces cambia de linea
            continue;
        std::wstring wline = converter.from_bytes(line);    //UTF-8 -> Unicode
        if (wline.size() < 3)                               // Si no hay al menos 3 code points, no hay trigramas
            continue;

        for (auto i = 0; i + 2 < wline.size(); ++i)         //garantiza que existan 3 caracteres desde i en adelante. 
        {
            std::wstring wtri = wline.substr(i, 3);         //Toma el trigrama como subcadena Unicode de longitud 3 a partir de i
            std::string tri = converter.to_bytes(wtri);     //Unicode -> UTF-8 para usar como clave std::string

            trigProfReturn[tri] += 1.0f;                    //incrementa la cantidad del trigrama tri en el mapa trigProfReturn
        }
    }
    
    return trigProfReturn;
}

/**
 * @brief Normalizes a trigram profile.
 *
 * @param trigramProfile The trigram profile.
 */
void normalizeTrigramProfile(TrigramProfile &trigramProfile)
{
    float norma = 0.0;
    for (auto& i : trigramProfile)
    {
        float adder = i.second;
        norma += (adder * adder);
    }

    norma = std::sqrtf(norma);

    for (auto &i : trigramProfile)
    {
        i.second /= norma;
    }

    return;
}

/**
 * @brief Calculates the cosine similarity between two trigram profiles
 *
 * @param textProfile The text trigram profile
 * @param languageProfile The language trigram profile
 * @return float The cosine similarity score
 */
float getCosineSimilarity(TrigramProfile &textProfile, TrigramProfile &languageProfile)
{
    float result = 0;

    for (auto &textIt : textProfile) // Recorro todos los trigramas del texto
    {
        auto langIt = languageProfile.find(textIt.first); // Me fijo si existe ese trigrama en el lenguaje
        if  (langIt != languageProfile.end())
        {
            result += (langIt->second * textIt.second); // Si existe, opero
        }
    }
    return result;
}

/**
 * @brief Identifies the language of a text.
 *
 * @param text A Text (vector of lines)
 * @param languages A list of Language objects
 * @return string The language code of the most likely language
 */
string identifyLanguage(const Text& text, LanguageProfiles& languages)
{
    TrigramProfile trig_prof = buildTrigramProfile(text);
    TrigramProfile& ref_trig_prof = trig_prof;
    if (trig_prof.empty() || languages.empty())
    {
        return "";  // sin datos para decidir
    }
    normalizeTrigramProfile(ref_trig_prof);

    float max = 0.0f;
    std::string lan_identified;
    float n;
    for (auto& lan : languages)         //recorre todos los lenguajes
    {
        if (lan.trigramProfile.empty()) continue;
        n = getCosineSimilarity(ref_trig_prof, lan.trigramProfile);
        if (n > max)                   //busca cual lenguaje tiene la mayor similitud coseno 
        {
            max = n;
            lan_identified = lan.languageCode;
        }
    }
    return lan_identified;
}

/*
* @brief Checks a list for a set number of characters
* 
* @param list The list 
* @param n The number
* @return 1 if it has less than n characters, 0 otherwise
*/
bool hasLessThanNChars(Text& text, int n)
{
    size_t total = 0;
    for (const auto& s : text) {
        total += s.size();

        if (total >= 3)
            return false;
    }
    return true;
}
