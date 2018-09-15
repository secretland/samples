#include <iostream>
#include <chrono>

#if defined(BOOST_ASIO_SSL)
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/array.hpp>

enum { max_length = 1024 };

class Client
{
public:
    Client(boost::asio::io_service& io_service,
           boost::asio::ssl::context& context,
           boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    : socket_(io_service, context)
    {
        socket_.set_verify_mode(boost::asio::ssl::verify_peer);
        socket_.set_verify_callback(
                boost::bind(&Client::verify_certificate, this, _1, _2));

        boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
                boost::bind(&Client::handle_connect, this,
                    boost::asio::placeholders::error));
    }

    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
    {
        // The verify callback can be used to check whether the certificate that is
        // being presented is valid for the peer. For example, RFC 2818 describes
        // the steps involved in doing this for HTTPS. Consult the OpenSSL
        // documentation for more details. Note that the callback is called once
        // for each certificate in the certificate chain, starting from the root
        // certificate authority.

        // In this example we will simply print the certificate's subject name.
        char subject_name[256];
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
        std::cout << "Verifying " << subject_name << "\n";

        return preverified;
    }

    void handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_handshake(boost::asio::ssl::stream_base::client,
                    boost::bind(&Client::handle_handshake, this,
                        boost::asio::placeholders::error));
        }
        else
        {
            std::cerr << "Connect failed: " << error.message() << "\n";
        }
    }

    void handle_handshake(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "Enter message: ";
            std::cin.getline(request_, max_length);
            size_t request_length = strlen(request_);

            boost::asio::async_write(socket_,
                    boost::asio::buffer(request_, request_length),
                    boost::bind(&Client::handle_write, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            std::cerr << "Handshake failed: " << error.message() << "\n";
        }
    }

    void handle_write(const boost::system::error_code& error,
            size_t bytes_transferred)
    {
        if (!error)
        {
            boost::asio::async_read(socket_,
                    boost::asio::buffer(reply_, bytes_transferred),
                    boost::bind(&Client::handle_read, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            std::cout << "Write failed: " << error.message() << "\n";
        }
    }

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (!error)
        {
            std::cout << "Reply: ";
            std::cout.write(reply_, bytes_transferred);
            std::cout << "\n";
        }
        else
        {
            std::cout << "Read failed: " << error.message() << "\n";
        }
    }

private:
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
    char request_[max_length];
    char reply_[max_length];
};

std::string get_password(std::size_t, boost::asio::ssl::context::password_purpose)
{
    return "123456";
}

static std::string ServerAddress;
static uint16_t ServerPort = 1000;
static uint32_t PacketSize = 0x10000;
static uint32_t PacketsNumber = 0x100;
static bool Help = false;

bool parse_cmdline(int argc, char** argv)
{
    char* const opt = "a:p:s:n:h";
    int c;
    while((c = getopt(argc, argv, opt)) != -1)
    {
        switch(c)
        {
        case 'a':
            ServerAddress = optarg;
            break;
        case 'p':
            ServerPort = atoi(optarg);
            break;
        case 's':
            PacketSize = atoi(optarg);
            break;
        case 'n':
            PacketsNumber = atoi(optarg);
            break;
        case 'h':
        default:
            Help = true;
            break;
        }
    }
    return true;
}

void print_help()
{
    std::cerr << "Usage: client -a <host address> -p <port number> -s <packet's size> -n <packets' number>";
    std::cerr << std::endl;
}

int main(int argc, char* argv[])
{
    try
    {
        OPENSSL_config("openssl.cnf");
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        std::cout << "Test SSL client. v1.0" << std::endl;
        parse_cmdline(argc, argv);
        if(Help || ServerAddress.empty())
        {
            print_help();
            return 1;
        }
        std::cout << "Server address: " << ServerAddress << std::endl;
        std::cout << "Server port: " << ServerPort << std::endl;
        std::cout << "Packet size: " << PacketSize << " bytes" << std::endl;
        std::cout << "Packets number: " << PacketsNumber << std::endl;

        boost::asio::io_service io_service;

        boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv1);
        ctx.set_password_callback(get_password);
        ctx.load_verify_file("cryptopro_certificate.pem");
        ctx.use_private_key_file("client_private_key_2012_512.pem", boost::asio::ssl::context_base::pem);
        ctx.use_certificate_file("client_certificate_2012_512.pem", boost::asio::ssl::context_base::pem);

#if defined(SYNC_CONNECTION)

        boost::system::error_code error;
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(io_service, ctx);
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ServerAddress), ServerPort);

        std::cout << "Attempt connection to [" << ServerAddress << "] ..." << std::endl;
        socket.lowest_layer().connect(endpoint, error);

        if(error)
        {
            std::cerr << "Connection failed." << std::endl;
            std::cerr << "boost::system::error_code. Category: " << error.category().name() 
                      << ". Value: " << std::hex << std::showbase << error.value() << std::dec 
                      << ". Error: " << error.message() << std::endl;
        }
        else
            std::cout << "Connection established!" << std::endl;

        socket.handshake(boost::asio::ssl::stream_base::client, error);
        if(error)
        {
            std::cerr << "Handshake failed." << std::endl;
            std::cerr << "boost::system::error_code. Category: " << error.category().name() 
                      << ". Value: " << std::hex << std::showbase << error.value() << std::dec 
                      << ". Error: " << error.message() << std::endl;
        }
        else
            std::cout << "Handshake successful!" << std::endl;

        std::vector<uint8_t> outputBuffer;
        outputBuffer.resize(PacketSize);
        std::size_t bytesSent = 0;
        auto timeStart = std::chrono::high_resolution_clock::now();
        uint32_t i = 0;
        for(; i < PacketsNumber; ++i)
        {
            bytesSent = socket.write_some(boost::asio::buffer(outputBuffer), error);
            if(error)
            {
                std::cerr << "Send data to station failed. Sent bytes: " << bytesSent << std::endl;
                std::cerr << "boost::system::error_code. Category: " << error.category().name() 
                          << ". Value: " << std::hex << std::showbase << error.value() << std::dec 
                          << ". Error: " << error.message() << std::endl;
                break;
            }
        }
        auto timeFinish = std::chrono::high_resolution_clock::now();
        std::cout << "Packets sent: " << i << std::endl;
        std::cout << "Duration time: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeFinish - timeStart).count() << " ms." << std::endl;
#else
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(argv[1], argv[2]);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        Client c(io_service, ctx, iterator);

        io_service.run();
#endif /* defined(SYNC_CONNECTION) */
    }
    catch(boost::system::error_code const& ec)
    {
        std::cerr << "Boost::system::error_code. Category: " << ec.category().name() << ". Value: " << std::hex << ec.value() << ". Error: " << ec.message() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
#else

#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>

#include <boost/asio.hpp>

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cerr << "There isn't server ip and port to connection!" << std::endl;
        return -1;
    }
    try
    {
        OPENSSL_config("openssl.cnf");
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        SSL_CTX * sslctx = SSL_CTX_new(TLSv1_method());

        if(SSL_CTX_use_certificate_file(sslctx, "client_certificate_2012_512.pem", SSL_FILETYPE_PEM) <= 0)
        {
            std::cerr << "code: " << ERR_get_error() << ". text: " << ERR_error_string(ERR_get_error(), NULL) << std::endl;
            exit(-2);
        }
        if(SSL_CTX_use_PrivateKey_file(sslctx, "client_private_key_2012_512.pem", SSL_FILETYPE_PEM) <= 0) 
        {
            std::cerr << "code: " << ERR_get_error() << ". text: " << ERR_error_string(ERR_get_error(), NULL) << std::endl;
            exit(-2);
        }

        SSL_CTX_set_options(sslctx, SSL_OP_NO_COMPRESSION);
        std::cout << "SSL_CTX_get_options() return: " << std::hex << SSL_CTX_get_options(sslctx) << std::endl;
        std::cout << "SSL_CTX_get_mode() return: " << std::hex << SSL_CTX_get_mode(sslctx) << std::endl;

        SSL* ssl = SSL_new(sslctx);

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::socket socket(io_service);;
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ServerAddress), ServerPort);
        socket.connect(endpoint);

        SSL_set_fd(ssl, socket.native());

        int retCode = SSL_connect(ssl);
        if(retCode != 1)
            std::cerr << "SSL_connect return error code: " << std::hex << retCode << std::endl;
        else
            std::cout << "SSL session established successful" << std::endl;

    }
    catch(std::exception const& error)
    {
        std::cerr << "Exception: " << error.what() << std::endl;
    }
    return 0;
}
#endif /* defined(BOOST_ASIO_SSL) */
