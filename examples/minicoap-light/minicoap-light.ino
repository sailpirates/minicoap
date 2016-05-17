#include "minicoap.h"
#ifdef WIRINGPI
    #include <wiringPi.h>
#endif // WIRINGPI

MiniCoAP coap;

// TODO: get rid of static content argument?
static unsigned char light = '1';
bool lightChanged = true;

void turnOffLight() {
    if (light=='1') {
        light = '0';
        lightChanged = true;
        #if defined(WIRINGPI) || defined(ARDUINO)
            digitalWrite(LED, LOW);
        #endif // WIRINGPI || ARDUINO
    }
}

void turnOnLight() {
    if (light=='0') {
        light = '1';
        lightChanged = true;
        #if defined(WIRINGPI) || defined(ARDUINO)
            digitalWrite(LED, HIGH);
        #endif // WIRINGPI || ARDUINO
    }
}

static const coap_endpoint_path_t pathLight = {1, {"light"}};
int getLight(const coap_packet_t *inpkt, coap_packet_t *outpkt) {
    return coap.coap_make_response(inpkt, outpkt, &light, sizeof(light), COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

int putLight(const coap_packet_t *inpkt, coap_packet_t *outpkt) {
    if (inpkt->payload.len == 1) {
        char pld = inpkt->payload.p[0];
        if (pld == '1') {
            turnOnLight();
        }
        else if (pld == '0'){
            turnOffLight();
        }
        return coap.coap_make_response(inpkt, outpkt, &light, sizeof(light), COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
    return coap.coap_make_response(inpkt, outpkt, NULL, 0, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_TEXT_PLAIN);
}

void updateResources() {
    #if defined(WIRINGPI) || defined(ARDUINO)
        if (digitalRead(BUTTON)==HIGH) {
            turnOnLight();
        }
        else if (digitalRead(BUTTON)==LOW) {
            turnOffLight();
        }
    #endif
}

void setup() {
    #ifdef WIRINGPI
        wiringPiSetup();
    #endif // WIRINGPI
    #if defined(WIRINGPI) || defined(ARDUINO)
        pinMode(LED, OUTPUT);
        digitalWrite(LED, HIGH); // FIXME: according to light
    #endif // WIRINGPI || ARDUINO
    #ifdef BUTTON // WIRINGPI BUTTON
        pinMode(BUTTON, INPUT);
    #endif
    coap.addEndpoint(COAP_METHOD_GET,getLight,&pathLight,&lightChanged,"ct=0;obs");
    coap.addEndpoint(COAP_METHOD_PUT,putLight,&pathLight); // TODO
}

// TODO: listen for hw button
void loop() {
    while(1)
    {
        updateResources();
        coap.answerForObservations();
        coap.answerForIncomingRequest();
    }
}

// for posix-based OSes compability:
#ifndef ARDUINO
    int main(int argc, char **argv)
    {
        setup();
        loop();
        return 0;
    }
#endif // !ARDUINO
