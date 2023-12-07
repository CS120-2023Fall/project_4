#ifndef TRANSMITTER_H
#define TRANSMITTER_H
#include <vector>
#include <string>
#include <iostream>
#include "utility.h"
#include "CRC.h"
#include <cmath>
#include <JuceHeader.h>

#define MAXIMUM_TOTAL_SECONDS 5//total time it will run most
#define PREAMBLE_SIZE 480 // the number of preamble
#define PACKET_DATA_SIZE 500 ////the number of packet
// I think PACKET_DATA_SIZE is the number of symbols in a packet.
// --fbz
#define END_FRAME_SIZE 48 //the number of empty zero between packets



const double pi = juce::MathConstants<double>::pi;
bool psk_mode = true;
//fsk_mode
bool test_for_repeat_preamble = false;
//
bool up_down = true;
constexpr const double freq_carrier_0 = 3000;

constexpr const double freq_carrier_1 = 5000;
//psk_mode
constexpr const double freq_carrier_psk = 10000;
constexpr const double freq_up_preamble = 1000;
constexpr const double freq_down_preamble = 1000;
constexpr const int default_sample_rate = 48000;
constexpr const int samples_per_bit = default_sample_rate / 1000 / 2;
// TRANSMITTER_CONSTANTS


class Transmitter {
public:
    Transmitter(const std::string &path, int _sample_rate) :sample_rate(_sample_rate) {
        bits = Read_bits(path);
        generate_carrier();
        generate_length();
        generate_preamble();
        generate_crc();
        generate_packet_sequences();
    }
    void generate_crc() {
        int size;
        unsigned char *data = unsigned_int_to_unsigned_char_star(from_bits_vector_to_unsigned_int(bits), size);
        CRC_8 = CRC::CalculateBits(data, size * 8, CRC::CRC_8());


    }
    void generate_preamble() {
        //int f0 = 1000;
        //double k = (10000 - f0) / time[PREAMBLE_SIZE - 1];
        //std::vector<float> t;
        //if (up_down) {
        //    for (int i = 0; i < PREAMBLE_SIZE / 2; i++) {
        //        preamble.push_back(cos(2 * pi * (f0 + k * time[i]) * time[i]));
        //        t.push_back(time[i]);
        //    }
        //    std::vector<double> p_r;
        //    for (int i = 0; i < preamble.size(); i++) {
        //        p_r.push_back(preamble[preamble.size() - 1 - i]);
        //    }
        //    for (int i = 0; i < p_r.size(); i++) {
        //        preamble.push_back(p_r[i]);
        //    }
        //    for (int i = PREAMBLE_SIZE - 1; i >= 0; i--) {
        //        preamble_reverse.push_back(preamble[i]);
        //    }
        //}
        //else {
        //    for (int i = 0; i < PREAMBLE_SIZE; i++) {
        //        preamble.push_back(cos(2 * pi * (f0 + k * time[i]) * time[i]));
        //        t.push_back(time[i]);
        //    }
        //}
        //preamble.clear();
        preamble = { 0.0f,  0.260934562803160f,  0.507569823429442f,  0.720910534598818f,  0.883016151010603f,  0.978645799875374f,  0.996891496796334f,  0.932635070783648f,  0.787650943175030f,  0.571181206068053f,  0.299835093750394f,  -0.00328617886950193f,  -0.310266866029823f,  -0.590794225408597f,  -0.815153176612418f,  -0.957534522633586f,  -0.999306543713813f,  -0.931842350217927f,  -0.758481857026729f,  -0.495249325245479f,  -0.170051183384019f,  0.179757729141529f,  0.511340154781374f,  0.781538695939956f,  0.952668141750426f,  0.998185715172346f,  0.907385004628948f,  0.688275114423472f,  0.367961591440144f,  -0.00985839465909892f,  -0.390260529829986f,  -0.714809816010999f,  -0.930644884754822f,  -0.999562671957063f,  -0.905534720590424f,  -0.659139900001489f,  -0.297744381977068f,  0.119114561653686f,  0.518851294842978f,  0.828260894501423f,  0.987238805952405f,  0.961241564591104f,  0.749850228155201f,  0.389251760891502f,  -0.0525547337307185f,  -0.487617404438996f,  -0.825179504591492f,  -0.990937154105352f,  -0.944347547502897f,  -0.689862769438167f,  -0.278864875529402f,  0.199117466845866f,  0.634922355281212f,  0.924507069341834f,  0.994475999924987f,  0.822073361626547f,  0.444007128776588f,  -0.0492728124429624f,  -0.533753469606811f,  -0.882501505847747f,  -0.999460098067586f,  -0.847389575832097f,  -0.461586667679532f,  0.0558360874798596f,  0.561249591094124f,  0.908761287602691f,  0.993010398869450f,  0.782903558404973f,  0.335148854451803f,  -0.218399797280335f,  -0.707880917988181f,  -0.977273696786789f,  -0.935374317627762f,  -0.589025230991454f,  -0.0459903590580718f,  0.516039255609672f,  0.906462037918370f,  0.987063775572624f,  0.723184056467672f,  0.201263907958821f,  -0.396303267472612f,  -0.853154857975455f,  -0.997543631464606f,  -0.769086928946605f,  -0.247161765658630f,  0.373048762597769f,  0.851435990752766f,  0.996066342348803f,  0.742558170393083f,  0.186219111861788f,  -0.449886492698291f,  -0.902254913276126f,  -0.976571811140622f,  -0.634075698751827f,  -0.0153349279018384f,  0.613530045858126f,  0.973658886063493f,  0.898441617363505f,  0.414327727998252f,  -0.263048828133159f,  -0.821449170853364f,  -0.995462451827537f,  -0.695393437713644f,  -0.0580233238475387f,  0.610931731711191f,  0.981263128191422f,  0.862720538291242f,  0.307141186444573f,  -0.408337126463353f,  -0.915935362914168f,  -0.946489113017158f,  -0.476099497141684f,  0.251405017377059f,  0.846224295881924f,  0.980624673724639f,  0.572978086801446f,  -0.157083576486024f,  -0.801614138208055f,  -0.990789419272197f,  -0.608326820112051f,  0.131069184452070f,  0.796344443616681f,  0.989564757619950f,  0.587253409513363f,  -0.174367300246237f,  -0.831925758267499f,  -0.975136409542769f,  -0.506625715192322f,  0.285170453150066f,  0.897477501587426f,  0.931444312534552f,  0.356731177157808f,  -0.455746423374272f,  -0.967606006447268f,  -0.830097808497854f,  -0.127810647004038f,  0.664886781520234f,  0.999865015406476f,  0.636613381814156f,  -0.177602195598857f,  -0.868207873561827f,  -0.936531454724829f,  -0.324808524246927f,  0.527253672923891f,  0.992213241104788f,  0.717867066186250f,  -0.0984259271619119f,  -0.842704109483208f,  -0.942897169194277f,  -0.313389195042925f,  0.563061442546998f,  0.998675014142943f,  0.641668096243593f,  -0.231207977123190f,  -0.919847281180361f,  -0.859938010540074f,  -0.0918833918126914f,  0.754901186350833f,  0.972139304525716f,  0.368979913584349f,  -0.550322218890278f,  -0.999735435910855f,  -0.584590395163117f,  0.342363232029077f,  0.970577730569156f,  0.738878674686395f,  -0.154919609365653f,  -0.911484397652296f,  -0.840930356212138f,  0.00109539470903012f,  0.844468762391972f,  0.903197422578657f,  0.113674820877989f,  -0.785622004028573f,  -0.937678478218057f,  -0.188371132341630f,  0.745485734487230f,  0.953662036253421f,  0.223741251944231f,  -0.729957666298296f,  -0.956581885804294f,  -0.220537174288366f,  0.741089038902239f,  0.947544569823485f,  0.178680069568311f,  -0.777421628594799f,  -0.923249501435092f,  -0.0973357922232584f,  0.833744724070304f,  0.876243347209666f,  -0.0240963559451455f,  -0.900356907940377f,  -0.795681423603992f,  0.184066197612434f,  0.962142398700131f,  0.668967782887897f,  -0.376095703648414f,  -0.998051411778172f,  -0.484745748687527f,  0.585478769478412f,  0.981890985996957f,  0.237597224404527f,  -0.786299260820495f,  -0.885573471686199f,  0.0656763891752464f,  0.940315466312446f,  0.685887440380955f,  -0.399318233077532f,  -0.999940006097469f,  -0.375080506280486f,  0.714043356939177f,  0.917685006183264f,  -0.0262864501070990f,  -0.938815375720775f,  -0.663248789574180f,  0.458669022110532f,  0.993764662669871f,  0.246100208128582f,  -0.822696566002258f,  -0.817052279371288f,  0.264105487987921f,  0.997145021259037f,  0.402328886444316f,  -0.732945426031138f,  -0.880951219407556f,  0.166811948758436f,  0.988264104844107f,  0.452818903032043f,  -0.709426642526106f,  -0.886588966108606f,  0.176524108526261f,  0.992617179623945f,  0.404333576943932f,  -0.760619345167526f,  -0.837355625300327f,  0.292511372454541f,  0.999805624109489f,  0.250344653753603f,  -0.867118719566577f,  -0.707106781186481f,  0.500000000000095f,  0.966490525965391f,  0.00876304726642015f,  -0.960635240997616f,  -0.530043118258514f,  0.667337783556289f,  0.902726709513690f,  -0.153837346326602f,  -0.990186593351195f,  -0.425263828179495f,  0.736660331319373f,  0.867663817114680f,  -0.206625762063978f,  -0.994125699079508f,  -0.416320630765625f,  0.728458527631164f,  0.884042262153464f,  -0.151672267783620f,  -0.982711634340057f,  -0.504735676166318f,  0.639986260389645f,  0.941797504530220f,  0.0131443606057688f,  -0.931045157220573f,  -0.673028717111254f,  0.446949224039999f,  0.995968681161589f,  0.284120371627160f,  -0.783584580949538f,  -0.870912516476629f,  0.122376701469828f,  0.963032842632671f,  0.623000300182734f,  -0.474171794347724f,  -0.996352154429032f,  -0.321698768862278f,  0.735177014705979f,  0.919417031542900f,  0.0219061461312981f,  -0.899401421016727f,  -0.775350490719248f,  0.241851037820260f,  0.980193153779535f,  0.603097317149782f,  -0.453795288040937f,  -0.999898611004247f,  -0.432191117881624f,  0.611798570864246f,  0.982303667480347f,  0.282019187102426f,  -0.721669241448561f,  -0.948242523652027f,  -0.163570912730220f,  0.792352018540146f,  0.913723584846904f,  0.0820622353059584f,  -0.832533079392359f,  -0.889609898569919f,  -0.0394239977910568f,  0.848550788686643f,  0.881985801780225f,  0.0361401608191937f,  -0.843293339213902f,  -0.892592403543206f,  -0.0722331031454263f,  0.815787189973438f,  0.918985678706274f,  0.147339937212675f,  -0.761330017034261f,  -0.954318911803946f,  -0.259876959864683f,  0.672218141494043f,  0.986887560824963f,  0.405335195061399f,  -0.539299761089289f,  -0.999826620898297f,  -0.573875496440963f,  0.354683671325922f,  0.971623442404112f,  0.747672018376474f,  -0.115851135519380f,  -0.878346276114961f,  -0.898922058493830f,  -0.168971640824312f,  0.698535483728226f,  0.990032916127990f,  0.475135930800084f,  -0.421294124855719f,  -0.977965028853617f,  -0.757767539649313f,  0.0569297398184638f,  0.824560253271995f,  0.953331881846564f,  0.352634463168961f,  -0.513221643668104f,  -0.990640495600806f,  -0.729208534449634f,  0.0689551184835414f,  0.813882216370112f,  0.967050586914158f,  0.426254981311505f,  -0.415324428553831f,  -0.961542996493309f,  -0.835554686263101f,  -0.133240758579829f,  0.657490800996163f,  0.999978402056876f,  0.670594571469852f,  -0.102785272835062f,  -0.805526033712479f,  -0.981051487104505f,  -0.523525560650787f,  0.266217855737201f,  0.884553726902378f,  0.947193887275739f,  0.423279992296054f,  -0.357754288630703f,  -0.918553223188015f,  -0.925754653496015f,  -0.383189381419533f,  0.382177368516162f,  0.921981963356395f,  0.929032633000616f,  0.407336971021987f,  -0.341333829169229f,  -0.896509078315790f,  -0.955295636070687f,  -0.493344887645397f,  0.232273552686823f,  0.830708122089793f,  0.988596386843338f,  0.630681474596418f,  -0.0503668470778207f,  -0.702444175502325f,  -0.997694697608496f,  -0.795017448861280f,  -0.202336766917267f,  0.485703550445042f,  0.936914920236955f,  0.941058743754463f,  0.503789747645430f,  -0.164651456068703f,  -0.754182331378057f,  -0.999495488662323f,  -0.797006508103629f,  -0.243976208463476f,  0.412332836640686f,  0.887600205295681f,  0.983114884617976f,  0.665704581297887f,  0.0744179950796774f,  -0.543903885597719f,  -0.939192089209671f,  -0.958163869807979f,  -0.600472782260215f,  -0.0120490526431103f,  0.577458238369411f,  0.945423435074209f,  0.959099266950456f,  0.620426416210866f,  0.0591168382550631f,  -0.519787397624033f,  -0.911934424609096f,  -0.984871091807331f,  -0.720150962735858f,  -0.214121908860009f,  0.359799222551801f,  0.811968453652077f,  0.998838731260331f,  0.866572581571363f,  0.463528997404539f,  -0.0798786393461312f,  -0.595204287375246f,  -0.928626782007386f,  -0.985805412553439f,  -0.756336178039858f,  -0.312348793073189f,  0.216261372049598f,  0.681089897463752f,  0.956900579716427f,  0.974156070196697f,  0.734434032756826f,  0.306098554510943f,  -0.196970070056995f,  -0.646695093250949f,  -0.933423315113008f,  -0.991657989543084f,  -0.813245271013769f,  -0.445969060340392f,  0.0208110011399829f,  0.478986763943272f,  0.827031314836421f,  0.991937995756039f,  0.943261461921734f,  0.696966133285704f,  0.309225343454417f,  -0.137581977925744f,  -0.553063050251204f,  -0.857129687076909f,  -0.994815561423430f,  -0.945065939715445f,  -0.722427082374600f,  -0.372032218720634f,  0.0405185172572512f,  0.442043066174674f,  0.764869656668365f,  0.957849770877934f,  0.993641932189625f,  0.871450312652145f,  0.615258576178865f,  0.268328945758315f,  -0.114763047050207f,  -0.477062492216293f,  -0.767684853819686f,  -0.948589794095355f,  -0.998890906725604f,  -0.916374423579302f,  -0.716340160156835f,  -0.428235752009667f,  -0.0907925757462084f,  0.253524838376701f,  0.563966355399080f,  0.806174637950872f,  0.955618918846567f,  0.999529679972491f,  0.937297261554293f,  0.779484371093492f,  0.545740974756332f,  0.261991852648834f,  -0.0427074090232570f,  -0.339273795996030f,  -0.601348349125164f,  -0.807468943688071f,  -0.942531745094014f,  -0.998500512333357f,  -0.974402909091977f,  -0.875714984914277f,  -0.713276041093788f,  -0.501896078262401f,  -0.258819045102461f };

    }

    void generate_length() {
        unsigned int size = bits.size();
        //the maximum bits is 2^15
        const int maximum_length_bits = 15;

        int l = 1 << (maximum_length_bits - 1);
        int current_shift = maximum_length_bits - 1;

        while (l) {
            bool bit = (bool)((l & size) >> current_shift);
            l = l >> 1;
            current_shift -= 1;
            length_bits.push_back(bit);
        }

    }
    void generate_carrier() {
        double step = 1.0 / static_cast<double>(sample_rate);//the total time step dt per sample
        for (int i = 0; i < sample_rate * MAXIMUM_TOTAL_SECONDS; i++) {
            time.push_back(i * step);
        }
        for (int i = 0; i < time.size(); i++) {
            //choose the psk modeluation mode
            if (psk_mode) {
                carrier_waves_0.push_back(cos(2 * pi * freq_carrier_psk * time[i] + pi));
                carrier_waves_1.push_back(cos(2 * pi * freq_carrier_psk * time[i]));
            }
            else {
                carrier_waves_0.push_back(cos(2 * pi * freq_carrier_0 * time[i]));
                carrier_waves_1.push_back(cos(2 * pi * freq_carrier_1 * time[i]));
            }

        }
    }//generate the carrier_waves
    void generate_packet_sequences() {
        //the total sequence 
        int size = bits.size();
        if (test_for_repeat_preamble) {
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < PREAMBLE_SIZE; j++) {
                    packet_sequences.push_back(preamble[j]);
                }
                for (int end_frame_index = 0; end_frame_index < END_FRAME_SIZE; end_frame_index++) {
                    packet_sequences.push_back(0);
                }
            }
        }//test for detetction of preamble
        else {
            for (int i = 0; i < size; i += PACKET_DATA_SIZE) {
                int start = i;
                int end = i + PACKET_DATA_SIZE;
                if (end >= size) {
                    end = size - 1;
                }
                for (int j = 0; j < PREAMBLE_SIZE; j++) {
                    packet_sequences.push_back(preamble[j]);
                }
                // push the preamble
                for (int bit_index = start; bit_index <= end; bit_index++) {
                    if (bits[bit_index] == 1) {
                        for (int j = 0; j < samples_per_bit; j++) {
                            int index = j + bit_index * samples_per_bit;
                            packet_sequences.push_back(carrier_waves_1[j]);
                        }
                    }
                    if (bits[bit_index] == 0) {
                        for (int j = 0; j < samples_per_bit; j++) {
                            int index = j + bit_index * samples_per_bit;
                            packet_sequences.push_back(carrier_waves_0[j]);
                        }
                    }
                }
                //modulation
                for (int k = 0; k < END_FRAME_SIZE; k++) {
                    packet_sequences.push_back(0);
                }

            }
        }
    }
    std::vector<bool> bits;
    std::vector<double>preamble;
    std::vector<double> preamble_reverse;
    std::vector<double>time;
    std::vector<double> carrier_waves_0;
    uint8_t CRC_8;
    std::vector<double> carrier_waves_1;
    std::vector<bool> length_bits;
    std::vector<bool> CRC_bits;
    int sample_rate;
    std::vector<double> packet_sequences;

};
// a transmitter_with_psk_ask


#define BITS_PER_SYMBOL 4// the symbol represent how many bit
float max_amplitude = 1;
bool check_crc = false;//the crc is 8 bit 
constexpr const unsigned int CRC_SYMBOL_SIZE = 8 / BITS_PER_SYMBOL;
 // !CRC_CONSTANTS



class Transmitter_with_wire {
public:
    Transmitter_with_wire() {

    }
    Transmitter_with_wire(const std::string &path, int _sample_rate) :sample_rate(_sample_rate) {
        bits = Read_bits_from_bin(path);//read the bits from a bin file
        seperation_num = BITS_PER_SYMBOL;//seperation_num is the ѹ�� ��λBit
        symbols = translate_from_bits_vector_to_unsigned_int_vector(bits, seperation_num);//bit_to_symbol
        generate_carrier();//generate carrier_0 and carrier_1
        generate_length();
        generate_preamble();//generate the preamble
        generate_crc();
        //generate_packet_sequences();//add the preamble and data together
        Write_symbols();// for debug
        generate_knots();//to determine the amplitude range 

    }
    void generate_knots() {
        unsigned int ask_num = 1 << (seperation_num - 1);
        amplitude_step = max_amplitude / (ask_num);
    }
    void Write_symbols() {
        FILE *file = fopen("symbols.txt", "w");
        for (auto &symbol : symbols) {
            fprintf(file, "%d ", symbol >> 1);
        }
        fclose(file);
    }
    std::vector<double> generate_one_packet_modulation() {

    }
    void generate_crc_one_packet() {

    }
    void generate_crc() {
        int size = bits.size();
        unsigned int data_bits_per_packet = BITS_PER_SYMBOL * PACKET_DATA_SIZE;

        for (int i = 0; i < size; i += data_bits_per_packet) {
            std::vector<bool>bits_slice;
            uint8_t crc_8;
            for (int j = i; j < i + data_bits_per_packet; j++) {
                bits_slice.push_back(bits[j]);
            }
            int data_size;
            unsigned char *data = unsigned_int_to_unsigned_char_star(from_bits_vector_to_unsigned_int(bits_slice), data_size);
            crc_8 = CRC::CalculateBits(data, data_bits_per_packet, CRC::CRC_8());
            std::vector<bool> crc_8_vector = from_uint8_t_to_bits_vector(crc_8);
            for (int i = 0; i <= 7; i++) {
                CRC_bits.push_back(crc_8_vector[i]);
            }


        }


    }
    void generate_preamble() {
        int f0 = 1000;
        double k = (10000 - f0) / time[PREAMBLE_SIZE - 1];
        std::vector<float> t;
        for (int i = 0; i < PREAMBLE_SIZE / 2; i++) {
            preamble.push_back(cos(2 * pi * (f0 + k * time[i]) * time[i]));
            t.push_back(time[i]);
        }
        std::vector<double> p_r;
        for (int i = 0; i < preamble.size(); i++) {
            p_r.push_back(preamble[preamble.size() - 1 - i]);
        }
        for (int i = 0; i < p_r.size(); i++) {
            preamble.push_back(p_r[i]);
        }
        for (int i = PREAMBLE_SIZE - 1; i >= 0; i--) {
            preamble_reverse.push_back(preamble[i]);
        }
        for (int i = 0; i < PREAMBLE_SIZE; i++) {
            preamble.push_back(cos(2 * pi * (f0 + k * time[i]) * time[i]));
            t.push_back(time[i]);
        }
        preamble.clear();
        preamble = { 0.0f,  0.260934562803160f,  0.507569823429442f,  0.720910534598818f,  0.883016151010603f,  0.978645799875374f,  0.996891496796334f,  0.932635070783648f,  0.787650943175030f,  0.571181206068053f,  0.299835093750394f,  -0.00328617886950193f,  -0.310266866029823f,  -0.590794225408597f,  -0.815153176612418f,  -0.957534522633586f,  -0.999306543713813f,  -0.931842350217927f,  -0.758481857026729f,  -0.495249325245479f,  -0.170051183384019f,  0.179757729141529f,  0.511340154781374f,  0.781538695939956f,  0.952668141750426f,  0.998185715172346f,  0.907385004628948f,  0.688275114423472f,  0.367961591440144f,  -0.00985839465909892f,  -0.390260529829986f,  -0.714809816010999f,  -0.930644884754822f,  -0.999562671957063f,  -0.905534720590424f,  -0.659139900001489f,  -0.297744381977068f,  0.119114561653686f,  0.518851294842978f,  0.828260894501423f,  0.987238805952405f,  0.961241564591104f,  0.749850228155201f,  0.389251760891502f,  -0.0525547337307185f,  -0.487617404438996f,  -0.825179504591492f,  -0.990937154105352f,  -0.944347547502897f,  -0.689862769438167f,  -0.278864875529402f,  0.199117466845866f,  0.634922355281212f,  0.924507069341834f,  0.994475999924987f,  0.822073361626547f,  0.444007128776588f,  -0.0492728124429624f,  -0.533753469606811f,  -0.882501505847747f,  -0.999460098067586f,  -0.847389575832097f,  -0.461586667679532f,  0.0558360874798596f,  0.561249591094124f,  0.908761287602691f,  0.993010398869450f,  0.782903558404973f,  0.335148854451803f,  -0.218399797280335f,  -0.707880917988181f,  -0.977273696786789f,  -0.935374317627762f,  -0.589025230991454f,  -0.0459903590580718f,  0.516039255609672f,  0.906462037918370f,  0.987063775572624f,  0.723184056467672f,  0.201263907958821f,  -0.396303267472612f,  -0.853154857975455f,  -0.997543631464606f,  -0.769086928946605f,  -0.247161765658630f,  0.373048762597769f,  0.851435990752766f,  0.996066342348803f,  0.742558170393083f,  0.186219111861788f,  -0.449886492698291f,  -0.902254913276126f,  -0.976571811140622f,  -0.634075698751827f,  -0.0153349279018384f,  0.613530045858126f,  0.973658886063493f,  0.898441617363505f,  0.414327727998252f,  -0.263048828133159f,  -0.821449170853364f,  -0.995462451827537f,  -0.695393437713644f,  -0.0580233238475387f,  0.610931731711191f,  0.981263128191422f,  0.862720538291242f,  0.307141186444573f,  -0.408337126463353f,  -0.915935362914168f,  -0.946489113017158f,  -0.476099497141684f,  0.251405017377059f,  0.846224295881924f,  0.980624673724639f,  0.572978086801446f,  -0.157083576486024f,  -0.801614138208055f,  -0.990789419272197f,  -0.608326820112051f,  0.131069184452070f,  0.796344443616681f,  0.989564757619950f,  0.587253409513363f,  -0.174367300246237f,  -0.831925758267499f,  -0.975136409542769f,  -0.506625715192322f,  0.285170453150066f,  0.897477501587426f,  0.931444312534552f,  0.356731177157808f,  -0.455746423374272f,  -0.967606006447268f,  -0.830097808497854f,  -0.127810647004038f,  0.664886781520234f,  0.999865015406476f,  0.636613381814156f,  -0.177602195598857f,  -0.868207873561827f,  -0.936531454724829f,  -0.324808524246927f,  0.527253672923891f,  0.992213241104788f,  0.717867066186250f,  -0.0984259271619119f,  -0.842704109483208f,  -0.942897169194277f,  -0.313389195042925f,  0.563061442546998f,  0.998675014142943f,  0.641668096243593f,  -0.231207977123190f,  -0.919847281180361f,  -0.859938010540074f,  -0.0918833918126914f,  0.754901186350833f,  0.972139304525716f,  0.368979913584349f,  -0.550322218890278f,  -0.999735435910855f,  -0.584590395163117f,  0.342363232029077f,  0.970577730569156f,  0.738878674686395f,  -0.154919609365653f,  -0.911484397652296f,  -0.840930356212138f,  0.00109539470903012f,  0.844468762391972f,  0.903197422578657f,  0.113674820877989f,  -0.785622004028573f,  -0.937678478218057f,  -0.188371132341630f,  0.745485734487230f,  0.953662036253421f,  0.223741251944231f,  -0.729957666298296f,  -0.956581885804294f,  -0.220537174288366f,  0.741089038902239f,  0.947544569823485f,  0.178680069568311f,  -0.777421628594799f,  -0.923249501435092f,  -0.0973357922232584f,  0.833744724070304f,  0.876243347209666f,  -0.0240963559451455f,  -0.900356907940377f,  -0.795681423603992f,  0.184066197612434f,  0.962142398700131f,  0.668967782887897f,  -0.376095703648414f,  -0.998051411778172f,  -0.484745748687527f,  0.585478769478412f,  0.981890985996957f,  0.237597224404527f,  -0.786299260820495f,  -0.885573471686199f,  0.0656763891752464f,  0.940315466312446f,  0.685887440380955f,  -0.399318233077532f,  -0.999940006097469f,  -0.375080506280486f,  0.714043356939177f,  0.917685006183264f,  -0.0262864501070990f,  -0.938815375720775f,  -0.663248789574180f,  0.458669022110532f,  0.993764662669871f,  0.246100208128582f,  -0.822696566002258f,  -0.817052279371288f,  0.264105487987921f,  0.997145021259037f,  0.402328886444316f,  -0.732945426031138f,  -0.880951219407556f,  0.166811948758436f,  0.988264104844107f,  0.452818903032043f,  -0.709426642526106f,  -0.886588966108606f,  0.176524108526261f,  0.992617179623945f,  0.404333576943932f,  -0.760619345167526f,  -0.837355625300327f,  0.292511372454541f,  0.999805624109489f,  0.250344653753603f,  -0.867118719566577f,  -0.707106781186481f,  0.500000000000095f,  0.966490525965391f,  0.00876304726642015f,  -0.960635240997616f,  -0.530043118258514f,  0.667337783556289f,  0.902726709513690f,  -0.153837346326602f,  -0.990186593351195f,  -0.425263828179495f,  0.736660331319373f,  0.867663817114680f,  -0.206625762063978f,  -0.994125699079508f,  -0.416320630765625f,  0.728458527631164f,  0.884042262153464f,  -0.151672267783620f,  -0.982711634340057f,  -0.504735676166318f,  0.639986260389645f,  0.941797504530220f,  0.0131443606057688f,  -0.931045157220573f,  -0.673028717111254f,  0.446949224039999f,  0.995968681161589f,  0.284120371627160f,  -0.783584580949538f,  -0.870912516476629f,  0.122376701469828f,  0.963032842632671f,  0.623000300182734f,  -0.474171794347724f,  -0.996352154429032f,  -0.321698768862278f,  0.735177014705979f,  0.919417031542900f,  0.0219061461312981f,  -0.899401421016727f,  -0.775350490719248f,  0.241851037820260f,  0.980193153779535f,  0.603097317149782f,  -0.453795288040937f,  -0.999898611004247f,  -0.432191117881624f,  0.611798570864246f,  0.982303667480347f,  0.282019187102426f,  -0.721669241448561f,  -0.948242523652027f,  -0.163570912730220f,  0.792352018540146f,  0.913723584846904f,  0.0820622353059584f,  -0.832533079392359f,  -0.889609898569919f,  -0.0394239977910568f,  0.848550788686643f,  0.881985801780225f,  0.0361401608191937f,  -0.843293339213902f,  -0.892592403543206f,  -0.0722331031454263f,  0.815787189973438f,  0.918985678706274f,  0.147339937212675f,  -0.761330017034261f,  -0.954318911803946f,  -0.259876959864683f,  0.672218141494043f,  0.986887560824963f,  0.405335195061399f,  -0.539299761089289f,  -0.999826620898297f,  -0.573875496440963f,  0.354683671325922f,  0.971623442404112f,  0.747672018376474f,  -0.115851135519380f,  -0.878346276114961f,  -0.898922058493830f,  -0.168971640824312f,  0.698535483728226f,  0.990032916127990f,  0.475135930800084f,  -0.421294124855719f,  -0.977965028853617f,  -0.757767539649313f,  0.0569297398184638f,  0.824560253271995f,  0.953331881846564f,  0.352634463168961f,  -0.513221643668104f,  -0.990640495600806f,  -0.729208534449634f,  0.0689551184835414f,  0.813882216370112f,  0.967050586914158f,  0.426254981311505f,  -0.415324428553831f,  -0.961542996493309f,  -0.835554686263101f,  -0.133240758579829f,  0.657490800996163f,  0.999978402056876f,  0.670594571469852f,  -0.102785272835062f,  -0.805526033712479f,  -0.981051487104505f,  -0.523525560650787f,  0.266217855737201f,  0.884553726902378f,  0.947193887275739f,  0.423279992296054f,  -0.357754288630703f,  -0.918553223188015f,  -0.925754653496015f,  -0.383189381419533f,  0.382177368516162f,  0.921981963356395f,  0.929032633000616f,  0.407336971021987f,  -0.341333829169229f,  -0.896509078315790f,  -0.955295636070687f,  -0.493344887645397f,  0.232273552686823f,  0.830708122089793f,  0.988596386843338f,  0.630681474596418f,  -0.0503668470778207f,  -0.702444175502325f,  -0.997694697608496f,  -0.795017448861280f,  -0.202336766917267f,  0.485703550445042f,  0.936914920236955f,  0.941058743754463f,  0.503789747645430f,  -0.164651456068703f,  -0.754182331378057f,  -0.999495488662323f,  -0.797006508103629f,  -0.243976208463476f,  0.412332836640686f,  0.887600205295681f,  0.983114884617976f,  0.665704581297887f,  0.0744179950796774f,  -0.543903885597719f,  -0.939192089209671f,  -0.958163869807979f,  -0.600472782260215f,  -0.0120490526431103f,  0.577458238369411f,  0.945423435074209f,  0.959099266950456f,  0.620426416210866f,  0.0591168382550631f,  -0.519787397624033f,  -0.911934424609096f,  -0.984871091807331f,  -0.720150962735858f,  -0.214121908860009f,  0.359799222551801f,  0.811968453652077f,  0.998838731260331f,  0.866572581571363f,  0.463528997404539f,  -0.0798786393461312f,  -0.595204287375246f,  -0.928626782007386f,  -0.985805412553439f,  -0.756336178039858f,  -0.312348793073189f,  0.216261372049598f,  0.681089897463752f,  0.956900579716427f,  0.974156070196697f,  0.734434032756826f,  0.306098554510943f,  -0.196970070056995f,  -0.646695093250949f,  -0.933423315113008f,  -0.991657989543084f,  -0.813245271013769f,  -0.445969060340392f,  0.0208110011399829f,  0.478986763943272f,  0.827031314836421f,  0.991937995756039f,  0.943261461921734f,  0.696966133285704f,  0.309225343454417f,  -0.137581977925744f,  -0.553063050251204f,  -0.857129687076909f,  -0.994815561423430f,  -0.945065939715445f,  -0.722427082374600f,  -0.372032218720634f,  0.0405185172572512f,  0.442043066174674f,  0.764869656668365f,  0.957849770877934f,  0.993641932189625f,  0.871450312652145f,  0.615258576178865f,  0.268328945758315f,  -0.114763047050207f,  -0.477062492216293f,  -0.767684853819686f,  -0.948589794095355f,  -0.998890906725604f,  -0.916374423579302f,  -0.716340160156835f,  -0.428235752009667f,  -0.0907925757462084f,  0.253524838376701f,  0.563966355399080f,  0.806174637950872f,  0.955618918846567f,  0.999529679972491f,  0.937297261554293f,  0.779484371093492f,  0.545740974756332f,  0.261991852648834f,  -0.0427074090232570f,  -0.339273795996030f,  -0.601348349125164f,  -0.807468943688071f,  -0.942531745094014f,  -0.998500512333357f,  -0.974402909091977f,  -0.875714984914277f,  -0.713276041093788f,  -0.501896078262401f,  -0.258819045102461f };

    }

    void generate_length() {
        unsigned int size = bits.size();
        //the maximum bits is 2^15
        const int maximum_length_bits = 15;

        int l = 1 << (maximum_length_bits - 1);
        int current_shift = maximum_length_bits - 1;

        while (l) {
            bool bit = (bool)((l & size) >> current_shift);
            l = l >> 1;
            current_shift -= 1;
            length_bits.push_back(bit);
        }

    }
    void generate_carrier() {
        double step = 1.0 / static_cast<double>(sample_rate);//the total time step dt per sample
        for (int i = 0; i < sample_rate * MAXIMUM_TOTAL_SECONDS; i++) {
            time.push_back(i * step);
        }
        for (int i = 0; i < time.size(); i++) {
            //choose the psk modeluation mode
            carrier_waves_0.push_back(cos(2 * pi * freq_carrier_psk * time[i] + pi));
            carrier_waves_1.push_back(cos(2 * pi * freq_carrier_psk * time[i]));

        }
    }//generate the carrier_waves
    unsigned int demodulate(double A) {//given the amplitude find the closest to match 
        //translate the waves to unsigned int

        unsigned int ask_num = 1 << (seperation_num - 1);
        unsigned int index = ask_num - 1;
        //first find the nearest
        float nearest = 0;
        for (int i = 0; i < ask_num; i++) {
            if (A > i * amplitude_step && A <= (i + 1) * amplitude_step) {
                if (A - i * amplitude_step < (i + 1) * amplitude_step - A) {
                    nearest = i * amplitude_step;
                    index = i - 1;
                    if (i == 0) {
                        index = 0;
                    }
                }
                else {
                    nearest = (i + 1) * amplitude_step;
                    index = i;
                }
                break;
            }
        }

        return index;
    }
    void generate_packet_sequences() {// add the head and modulate the data
        //the total sequence 
        int size = symbols.size();
        unsigned int ask_num = 1 << (seperation_num - 1);// if 4 bits then 8 amplitude
        for (int i = 0; i < size; i += PACKET_DATA_SIZE) {

            int start = i;
            int end = i + PACKET_DATA_SIZE;
            if (end >= size) {
                end = size - 1;
            }
            for (int j = 0; j < PREAMBLE_SIZE; j++) {
                packet_sequences.push_back(preamble[j]);
            }
            // push the preamble
          //then the crc

            for (int symbol_index = start; symbol_index <= end; symbol_index++) {
                unsigned int symbol = symbols[symbol_index];
                auto carrier = (symbol & 1) == 1 ? carrier_waves_1 : carrier_waves_0;//using the last bit to determine psk
                float amplitude = max_amplitude * float((symbol >> 1) + 1) / ask_num;
                for (int j = 0; j < samples_per_bit; j++) {
                    packet_sequences.push_back(amplitude * carrier[j]);
                }
            }
            //modulation

        }
    }
    int seperation_num;
    std::vector<bool> bits;
    std::vector<unsigned int > symbols;
    std::vector<double>preamble;
    std::vector<double> preamble_reverse;
    std::vector<double>time;
    std::vector<double> carrier_waves_0;
    uint8_t CRC_8;
    std::vector<bool> CRC_bits;
    std::vector<double> carrier_waves_1;
    std::vector<bool> length_bits;
    int sample_rate;
    std::vector<double> sequence;
    std::vector<double> packet_sequences;
    int transmitted_packet;
    float amplitude_step;
    std::vector<double>knots;
    std::vector<double> transmittion_buffer;
};
Transmitter_with_wire default_trans_wire("INPUT.bin", default_sample_rate);

Transmitter default_trans("INPUT.txt", default_sample_rate);


#endif
