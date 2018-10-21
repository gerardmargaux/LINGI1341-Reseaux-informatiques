#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>
#include <CUnit/CUCurses.h>


int main(int argc, char *argv[])
{
    CU_pSuite pSuite = NULL;
    CU_initialize_registry(); // On initialise le registre

    if (CUE_SUCCESS != CU_initialize_registry()){ // On teste si l'initialisation s'est déroulee correctement
        CU_cleanup_registry(); // On libere le registre qui a ete cree
        return CU_get_error(); // Si il y a eu un problème on retourne le code d'erreur
    }

    pSuite = CU_add_suite("Suite de tests", 0, 0);
    else if ((NULL == CU_add_test(pSuite, "Test assert ptr not null", test_pkt_new)) ||
             (NULL == CU_add_test(pSuite, "Test assert ptr not null", test_ajout_buffer)) ||
             (NULL == CU_add_test(pSuite, "Test assert ptr null", test_retire_buffer)) ||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_get_type)) ||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_set_type)) ||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_get_tr))
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_set_tr))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_get_length))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_set_length))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_get_window))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_set_window))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_get_seqnum))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_set_seqnum))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_get_payload))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_set_payload))||
             (NULL == CU_add_test(pSuite, "Test assert equal", test_pkt_encode))){
        CU_cleanup_registry(); // On libere le registre qui a ete cree
        return CU_get_error(); // Si il y a eu un problème on retourne le code d'erreur
    }

    CU_basic_run_tests(); // On fait tourner les tests que nous avons cree
    CU_basic_show_failures(CU_get_failure_list()); // On affiche les erreurs
    CU_cleanup_registry(); // On libere le registre qui a ete cree

    return CU_get_error();
}

// Test de la fonction pkt_new()
void test_pkt_new(void){
    pkt_t * nouveau_pkt = pkt_new();
    CU_ASSERT_PTR_NOT_NULL(nouveau_pkt);
    pkt_del(nouveau_pkt)
}

// Test de la fonction ajout_buffer()
void test_ajout_buffer(void){
    pkt_t * nouveau_pkt = pkt_new();
    pkt_t ** buffer = (pkt_t **)malloc(32*sizeof(pkt_t));
    uint8_t min_window = 4;
    ajout_buffer(nouveau_pkt, buffer, min_window);
    CU_ASSERT_PTR_NOT_NULL(*buffer));
    free(buffer);
    pkt_del(nouveau_pkt);
}

// Test de la fonction retire_buffer()
void test_retire_buffer(void){
    pkt_t * nouveau_pkt = pkt_new();
    pkt_t ** buffer = (pkt_t **)malloc(32*sizeof(pkt_t));
    uint8_t seqnum = 10;
    uint8_t min_window = 4;
    ajout_buffer(nouveau_pkt, buffer, min_window);
    retire_buffer(buffer, seqnum);
    CU_ASSERT_PTR_NULL(*buffer);
    free(buffer);
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_get_type()
void test_pkt_get_type(void){
    pkt_t * nouveau_pkt = pkt_new();
    nouveau_pkt->type = PTYPE_ACK;
    CU_ASSERT_EQUAL(PTYPE_ACK, pkt_get_type(nouveau_pkt));
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_set_type()
void test_pkt_set_type(void){
    pkt_t * nouveau_pkt = pkt_new();
    pkt_status_code err_code = pkt_set_type(nouveau_pkt, PTYPE_DATA);
    CU_ASSERT_EQUAL(PKT_OK, err_code);
    err_code = pkt_set_type(nouveau_pkt, 6);
    CU_ASSERT_NOT_EQUAL(PKT_OK, err_code);
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_get_tr()
void test_pkt_get_tr(void){
    pkt_t * nouveau_pkt = pkt_new();
    nouveau_pkt->tr = 1;
    CU_ASSERT_EQUAL(1, pkt_get_tr(nouveau_pkt));
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_set_tr()
void test_pkt_set_tr(void){
    pkt_t * nouveau_pkt = pkt_new();
    pkt_status_code err_code = pkt_set_tr(nouveau_pkt, 1);
    CU_ASSERT_EQUAL(PKT_OK, err_code);
    err_code = pkt_set_tr(nouveau_pkt, 2);
    CU_ASSERT_NOT_EQUAL(PKT_OK, err_code);
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_get_length()
void test_pkt_get_length(void){
    pkt_t * nouveau_pkt = pkt_new();
    nouveau_pkt->length = 15;
    CU_ASSERT_EQUAL(15, pkt_get_length(nouveau_pkt));
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_set_length()
void test_pkt_set_length(void){
    pkt_t * nouveau_pkt = pkt_new();
    pkt_status_code err_code = pkt_set_length(nouveau_pkt, 12);
    CU_ASSERT_EQUAL(PKT_OK, err_code);
    err_code = pkt_set_length(nouveau_pkt, 520);
    CU_ASSERT_NOT_EQUAL(PKT_OK, err_code);
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_get_window()
void test_pkt_get_window(void){
    pkt_t * nouveau_pkt = pkt_new();
    nouveau_pkt->window = 3;
    CU_ASSERT_EQUAL(3, pkt_get_window(nouveau_pkt));
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_set_window()
void test_pkt_set_window(void){
    pkt_t * nouveau_pkt = pkt_new();
    pkt_status_code err_code = pkt_set_window(nouveau_pkt, 5);
    CU_ASSERT_EQUAL(PKT_OK, err_code);
    err_code = pkt_set_window(nouveau_pkt, 33);
    CU_ASSERT_NOT_EQUAL(PKT_OK, err_code);
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_get_seqnum()
void test_pkt_get_seqnum(void){
    pkt_t * nouveau_pkt = pkt_new();
    nouveau_pkt->seqnum = 108;
    CU_ASSERT_EQUAL(108, pkt_get_seqnum(nouveau_pkt));
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_set_seqnum()
void test_pkt_set_seqnum(void){
    pkt_t * nouveau_pkt = pkt_new();
    pkt_status_code err_code = pkt_set_seqnum(nouveau_pkt, 120);
    CU_ASSERT_EQUAL(PKT_OK, err_code);
    err_code = pkt_set_seqnum(nouveau_pkt, 256);
    CU_ASSERT_NOT_EQUAL(PKT_OK, err_code);
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_get_payload()
void test_pkt_get_payload(void){
    pkt_t * nouveau_pkt = pkt_new();
    char * buffer = "Je suis le payload";
    nouveau_pkt->payload = buffer;
    CU_ASSERT_STRING_EQUAL(buffer, pkt_get_payload(nouveau_pkt));
    free(buffer);
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_set_payload()
void test_pkt_set_payload(void){
    pkt_t * nouveau_pkt = pkt_new();
    char * buffer = "Je suis le payload";
    const uint8_t length = 19;
    pkt_status_code err_code = pkt_set_payload(nouveau_pkt, buffer, 19);
    CU_ASSERT_EQUAL(PKT_OK, err_code);
    free(buffer);
    pkt_del(nouveau_pkt);
}

// Test de la fonction pkt_encode()
void test_pkt_encode(void){
    pkt_t * nouveau_pkt = pkt_new();
	  char * buffer = "Je suis le payload";
	  char * data = (char *)malloc(528);
  	pkt_status_code err_code = pkt_set_payload(nouveau_pkt, buffer 19);
	  err_code = pkt_encode(nouveau_pkt, data, 528);
	  CU_ASSERT_EQUAL(err_code, PKT_OK);
    size = sizeof(data);
    pkt_t * pkt_decode = pkt_new();
	  err_code = pkt_decode(data, size , pkt_decode);
	  CU_ASSERT_EQUAL(err_code, PKT_OK);
    free(data);
    free(buffer);
	  pkt_del(nouveau_pkt);
    pkt_del(pkt_encode);
}
