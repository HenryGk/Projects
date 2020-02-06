/*
crack program takes input of hashed password and return the password
assume that each password has been hashed with C’s DES-based (not MD5-based) crypt function.
assume that each password is no longer than five (5) characters.
assume that each password is composed entirely of alphabetical characters (uppercase and/or lowercase).
*/

#include <stdio.h>
#include <crypt.h>
#include <string.h>

// Prototypes
int cracker(int index, char crack[6], char salt[3], char *hash, char *alphabet, int i);
int rcracker(int maxIndex, int index, char crack[6], char salt[3], char *hash, char *alphabet);

int main(int argc, char * argv[])
{
    // if input more than one arguments, return
    if (argc != 2)
    {
        printf("Usage: ./crack hash");
        return 1;
    }

    // get hash code from the argument, save it as char * hash
    char *hash = argv[1];
    // create crack which will be password trial combination of alphabet
    char crack[6];
    // get the salt from hash code (first 2 letters)
    char salt[3];
    salt[0] = hash[0];
    salt[1] = hash[1];
    salt[2] = '\0';

    // create char array of alphabet lower and upper case
    char alphabet[52] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

    // char array of most used english words
    char * commonWords[432] = {"login", "LOGIN", "hello", "HELLO", "abcd", "ABCD", "abc", "ABC", "lol", "LOL", "yea", "YEA", "qwerty", "QWERTY", "hi", "HI", "man", "MAN", "wow", "WOW", "the", "name", "of", "very", "to", "and", "just", "a", "form", "in", "much", "is", "great", "it", "think", "you", "say", "that", "help", "he", "low", "was", "line", "for", "on", "turn", "are", "cause", "with", "same", "as", "mean", "I", "his", "move", "they", "right", "be", "boy", "at", "old", "one", "too", "have", "does", "this", "tell", "from", "or", "set", "had", "three", "by", "want", "hot", "air", "but", "well", "some", "also", "what", "play", "there", "small", "we", "end", "can", "put", "out", "home", "other", "read", "were", "hand", "all", "port", "your", "large", "when", "spell", "up", "add", "use", "even", "word", "land", "how", "here", "said", "must", "an", "big", "each", "high", "she", "such", "which", "do", "act", "their", "why", "time", "ask", "if", "men", "will", "way", "went", "about", "light", "many", "kind", "then", "off", "them", "need", "would", "house", "write", "like", "try", "so", "us", "these", "again", "her", "long", "point", "make", "thing", "world", "see", "near", "him", "build", "two", "self", "has", "earth", "look", "more", "head", "day", "stand", "could", "own", "go", "page", "come", "did", "my", "found", "sound", "no", "most", "grow", "study", "who", "over", "learn", "know", "plant", "water", "cover", "than", "food", "call", "sun", "first", "four", "may", "let", "down", "keep", "side", "eye", "been", "never", "now", "last", "find", "door", "any", "new", "city", "work", "tree", "part", "cross", "take", "since", "get", "hard", "place", "start", "made", "might", "live", "story", "where", "saw", "after", "far", "back", "sea", "draw", "only", "left", "round", "late", "man", "run", "year", "don't", "came", "while", "show", "press", "every", "close", "good", "night", "me", "real", "give", "life", "our", "few", "under", "open", "ten", "seem", "next", "vowel", "white", "war", "begin", "lay", "got", "walk", "slow", "ease", "paper", "love", "often", "money", "music", "serve", "those", "both", "road", "mark", "map", "book", "rule", "until", "mile", "pull", "river", "cold", "car", "feet", "voice", "care", "fall", "power", "group", "town", "carry", "fine", "took", "rain", "fly", "eat", "unit", "room", "lead", "cry", "began", "dark", "idea", "fish", "note", "wait", "north", "plan", "once", "base", "star", "hear", "box", "horse", "noun", "cut", "field", "sure", "rest", "watch", "color", "able", "face", "pound", "wood", "done", "main", "drive", "plain", "stood", "girl", "usual", "front", "young", "teach", "ready", "week", "above", "final", "ever", "gave", "red", "green", "list", "oh", "quick", "feel", "talk", "sleep", "bird", "warm", "soon", "free", "body", "dog", "mind", "pose", "leave", "clear", "song", "tail", "state", "fact", "black", "inch", "short", "lot", "class", "wind", "stay", "wheel", "full", "force", "ship", "blue", "area", "half", "rock", "order", "deep", "fire", "moon", "south", "foot", "piece", "yet", "told", "busy", "knew", "test", "pass", "farm", "boat", "top", "whole", "gold", "king", "size", "plane", "heard", "age", "best", "dry", "hour", "laugh", "true", "ago", "ran", "am", "check", "game", "step", "shape", "early", "yes", "hold", "hot", "west", "miss", "heat", "reach", "snow", "fast", "bed", "five", "bring", "sing", "sit", "six", "fill", "table", "east", "less", "among"};

    // optimisation, first try to crypt all most common words for password
    // loop through commonWords, crypt and compare with hash, if match return
    for (int i = 0; i < 432; i ++)
    {
        char * myhash = "";
        char * pass = commonWords[i];
        myhash = crypt(pass, salt);

        if (strcmp(myhash, hash) == 0)
        {
            printf("%s\n", pass);
            return 1;
        }
    }

    /*
    call recursive rcracker function 5 times,
    first loop through 1st letter, then all combinations of 1st and 2nd,
    then 1st, 2nd and 3rd, and so on until we check all combinations of 5 letters
    */
    for (int maxIndex = 0; maxIndex < 5; maxIndex ++)
    {
        if (rcracker(maxIndex, 0, crack, salt, hash, alphabet) == 1)
        {
            return 1;
        }
    }
}

/* rcracker is looping through all total number of alphabet array
 * and calls cracker function with each i, and index
 * then recursively calling itself with index + 1 (crack's next element)
 * putting crack's first element all alphabetical combination, then 2nd element and so on for all 5 elements
 * ex: a, b, c ..., then aa, ab, ac... then ba, bb, bc...; aaa, aab, aac ...; baa, bab, bac... and so on
 * parameters: maxIndex - number of letters of crack, index - crack's index, crack, hash, salt, alphabet, i - iterator index
 */
int rcracker(int maxIndex, int index, char crack[6], char salt[3], char * hash, char * alphabet)
{
    if (index <= maxIndex)
    {
        int crackResult;
        for (int i = 0; i < 52; i++)
        {
            crackResult = cracker(index, crack, salt, hash, alphabet, i);
            if (crackResult == 1)
            {
                return 1;
            }
            // if maxIndex is not equel to index yet, call self recursively
            if (index != maxIndex)
            {
                if (rcracker(maxIndex, index + 1, crack, salt, hash, alphabet))
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
cracker creates crack, the crack's index element is alphabet's i-th element
and tries to crypt crack with given salt
then compare the crypt which given hash,
if matching return the crack
parameters: index - crack's index, crack, hash, salt, alphabet, i - iterator index
*/
int cracker(int index, char crack[6], char salt[3], char *hash, char *alphabet, int i)
{
    crack[index] = alphabet[i];
    crack[index + 1] = '\0';

    char *myhash = "";
    myhash = crypt(crack, salt);
    // if myhash and hash are same, return crack for that hash
    if (strcmp(myhash, hash) == 0)
    {
        printf("%s\n", crack);
        return 1;
    }
    return 0;
}



