\# 🔍 PDF Search Engine \& Inverted Index



\## 📌 About the Project

This project is a \*\*Search Engine\*\* built in C++ that utilizes an \*\*Inverted File\*\* architecture to index and retrieve information from PDF documents. The vocabulary of the inverted file is managed using a \*\*Trie data structure\*\* to efficiently map words to their exact occurrences and positions across multiple files.



\*\*Academic Context:\*\* Practical Project #3 developed for the Data Structures and Algorithms course at the Federal University of Itajubá (UNIFEI).



\## ⚙️ How it Works



\### 1. File Processing \& Normalization

The system scans a specified directory for PDF files and converts them into processed `TXT` files. During this phase, the text is heavily normalized:

\* Converted entirely to lowercase.

\* All accents and diacritics are removed.

\* Non-alphabetical characters are replaced with empty spaces to maintain the original word positioning.



\### 2. The Inverted Index (Trie)

An inverted file consists of a vocabulary (all distinct words) and occurrences (lists of positions where each word appears). This project implements a \*\*Trie (Prefix Tree)\*\* to store the vocabulary, mapping each recognized word to the documents it appears in, along with its exact positional data. 



\### 3. Querying \& Relevance Ranking

The search interface allows users to perform:

\* \*\*Word Search:\*\* Retrieves all occurrences of a specific word, sorting the results by the highest incidence (relevance).

\* \*\*Phrase Search:\*\* Retrieves documents containing all words in a phrase. The results are ranked based on a strict relevance algorithm:

&nbsp; \* \*High Relevance:\* Documents where words appear in the exact sequence and close proximity.

&nbsp; \* \*Low Relevance:\* Documents where words are present but out of order or scattered.



\## 🛠️ Technologies \& Dependencies

\* \*\*C++11 or higher\*\*

\* \*\*Poppler-cpp:\*\* Library used for parsing and extracting text from PDF files.

\* \*\*Standard Template Library (STL):\*\* Used for dynamic data structures and memory management.

