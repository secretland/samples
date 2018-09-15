#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

static uint16_t ServerPort = 1000;
static uint32_t PacketSize = 0x10000;
static bool Help = false;

class Session
{
public:
    Session(boost::asio::io_service& io_service,
            boost::asio::ssl::context& context)
    : socket_(io_service, context),
      data_(new char[PacketSize + 1])
    {
    }

    ~Session()
    {
        delete[] data_;
    }

    ssl_socket::lowest_layer_type& socket()
    {
        return socket_.lowest_layer();
    }

    void start()
    {
        socket_.async_handshake(boost::asio::ssl::stream_base::server,
                                boost::bind(&Session::handle_handshake, this,
                                            boost::asio::placeholders::error));
    }

    void handle_handshake(boost::system::error_code const& error)
    {
        if (!error)
        {
            std::cout << "Handshake succeful!" << std::endl;
            socket_.async_read_some(boost::asio::buffer(data_, PacketSize),
                                    boost::bind(&Session::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            std::cerr << "Handshake failed!" << std::endl;
            std::cerr << "boost::system::error_code. Category: " << error.category().name() 
                      << ". Value: " << std::hex << std::showbase << error.value() << std::dec 
                      << ". Error: " << error.message() << std::endl;
            delete this;
        }
    }

    void handle_read(boost::system::error_code const& error, size_t bytes_transferred)
    {
        if (!error)
        {
            std::cout << "Received data: " << bytes_transferred << " bytes." << std::endl;
            /*boost::asio::async_write(socket_,
                    boost::asio::buffer(data_, bytes_transferred),
                    boost::bind(&Session::handle_write, this,
                        boost::asio::placeholders::error));*/
            socket_.async_read_some(boost::asio::buffer(data_, PacketSize),
                                    boost::bind(&Session::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            std::cerr << "Recv data failed!" << std::endl;
            std::cerr << "boost::system::error_code. Category: " << error.category().name() 
                      << ". Value: " << std::hex << std::showbase << error.value() << std::dec 
                      << ". Error: " << error.message() << std::endl;
            delete this;
        }
    }

    void handle_write(boost::system::error_code const& error)
    {
        if(!error)
        {
            socket_.async_read_some(boost::asio::buffer(data_, PacketSize/*max_length*/),
                                    boost::bind(&Session::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            delete this;
        }
    }

private:
    ssl_socket socket_;
    //enum { max_length = 0x10000 };
    char* data_;
};

class Server
{
public:
    Server(boost::asio::io_service& io_service, boost::asio::ssl::context& context, unsigned short port)
        : io_service_(io_service),
          context_(context),
          acceptor_(io_service,
                    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {
        //context_.set_password_callback(boost::bind(&Server::get_password, this));
        std::cout << "Server started!" << std::endl;
        start_accept();
    }

    //std::string get_password() const
    //{
        //return "123456";
    //}

    void start_accept()
    {
        std::cout << "Waiting incoming connection..." << std::endl;
        Session* new_session = new Session(io_service_, context_);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&Server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(Session* new_session, boost::system::error_code const& error)
    {
        if(!error)
        {
            std::cout << "Incoming connection. IP: " << new_session->socket().remote_endpoint().address().to_string();
            new_session->start();
        }
        else
        {
            delete new_session;
        }
        start_accept();
    }

private:
    boost::asio::io_service& io_service_;
    boost::asio::ssl::context& context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};


std::string get_password(std::size_t, boost::asio::ssl::context::password_purpose)
{
    return "123456";
}

bool parse_cmdline(int argc, char** argv)
{
    char* const opt = "p:s:h";
    int c;
    while((c = getopt(argc, argv, opt)) != -1)
    {
        switch(c)
        {
        case 'p':
            ServerPort = atoi(optarg);
            break;
        case 's':
            PacketSize = atoi(optarg);
            break;
        case 'h':
        default:
            Help = true;
            break;
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
    try
    {
        OPENSSL_config("openssl.cnf");
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        std::cout << "Test SSL server. v1.0" << std::endl;
        if(!parse_cmdline(argc, argv) || Help)
        {
            std::cerr << "Usage: ssl_server -p <port> -s <packet's size>" << std::endl;
            return 1;
        }

        std::cout << "Server port: " << ServerPort << std::endl;
        std::cout << "Packet size: " << PacketSize << " bytes" << std::endl;

        boost::asio::io_service io_service;
        boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv1);
        ctx.set_options(boost::asio::ssl::context::no_sslv2);
        ctx.set_password_callback(get_password);
        ctx.load_verify_file("cryptopro_certificate.pem");
        ctx.use_private_key_file("server_private_key_2012_512.pem", boost::asio::ssl::context_base::pem);
        ctx.use_certificate_file("server_certificate_2012_512.pem", boost::asio::ssl::context_base::pem);

        Server s(io_service, ctx, ServerPort);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
