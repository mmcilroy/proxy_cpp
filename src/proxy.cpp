#include <boost/asio.hpp>
#include <iostream>

void hex_dump( void *addr, int len ) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;
    for( i=0; i<len; i++ ) {
        if( (i%16) == 0 ) {
            if( i != 0 ) {
                printf( "  %s\n", buff );
            }
            printf( "  %04x ", i );
        }
        printf (" %02x", pc[i]);
        if( ( pc[i] < 0x20 ) || ( pc[i] > 0x7e ) ) {
            buff[ i%16 ] = '.';
        } else {
            buff[ i%16 ] = pc[i];
        }
        buff[ ( i%16 ) + 1 ] = '\0';
    }
    while( ( i%16 ) != 0 ) {
        printf( "   " ); i++;
    }
    printf( "  %s\n", buff );
}

using boost::asio::ip::tcp;

class proxy {
public:
    proxy( const std::string& remote_host, short remote_port, short local_port ) :
        acceptor_( io_, tcp::endpoint( tcp::v4(), local_port ) ),
        remote_sock_( io_ ),
        local_sock_( io_ ),
        remote_host_( remote_host ),
        remote_port_( remote_port ) {
        accept_local();
    }

    void start() {
        io_.run();
    }

private:
    void connect_remote() {
        std::cout << "connecting to " << remote_host_ << ":" << remote_port_ << std::endl;
        tcp::resolver resolver( io_ );
        auto endpoint = resolver.resolve( { remote_host_, std::to_string( remote_port_ ) } );
        boost::asio::async_connect( remote_sock_, endpoint,
            [ this ]( boost::system::error_code ec, tcp::resolver::iterator ) {
                if( !ec ) {
                    read_remote();
                } else {
                    std::cout << "failed to connect to remote host" << std::endl;
                }
            } );
    }

    void accept_local() {
        std::cout << "accept starting" << std::endl;
        if( local_sock_.is_open() ) {
            local_sock_.close();
        }
        if( remote_sock_.is_open() ) {
            remote_sock_.close();
        }
        acceptor_.async_accept( local_sock_,
            [ this ]( boost::system::error_code ec ) {
                if( !ec ) {
                    std::cout << "accepted" << std::endl;
                    connect_remote();
                    read_local();
                } else {
                    std::cout << "accept failed: " << ec.message() << std::endl;
                }
            } );
    }

    void read_remote() {
        remote_sock_.async_read_some( boost::asio::buffer( remote_buf_, max_length ),
            [ this ]( boost::system::error_code ec, std::size_t length ) {
                std::cout << length << " bytes from remote" << std::endl;
                if( !ec ) {
                    if( length ) {
                        hex_dump( remote_buf_, length );
                    }
                    boost::asio::async_write( local_sock_, boost::asio::buffer( remote_buf_, length ),
                        [ this ]( boost::system::error_code ec, std::size_t length ) {
                            if( !ec ) {
                                read_remote();
                            } else {
                                std::cout << "local connection closed" << std::endl;
                            }
                        } );
                } else {
                    std::cout << "remote connection closed" << std::endl;
                }
            } );
    }

    void read_local() {
        local_sock_.async_read_some( boost::asio::buffer( local_buf_, max_length ),
            [ this ]( boost::system::error_code ec, std::size_t length ) {
                std::cout << length << " bytes from local" << std::endl;
                if( !ec ) {
                    if( length ) {
                        hex_dump( local_buf_, length );
                    }
                    boost::asio::async_write( remote_sock_, boost::asio::buffer( local_buf_, length ),
                        [ this ]( boost::system::error_code ec, std::size_t length ) {
                            if( !ec ) {
                                read_local();
                            } else {
                                std::cout << "remote connection closed" << std::endl;
                            }
                        } );
                } else {
                    std::cout << "local connection closed" << std::endl;
                    accept_local();
                }
            } );
    }

    boost::asio::io_service io_;
    tcp::acceptor acceptor_;
    tcp::socket remote_sock_;
    tcp::socket local_sock_;

    std::string remote_host_;
    short remote_port_;

    enum { max_length = 1024 };
    char remote_buf_[ max_length ];
    char local_buf_[ max_length ];
};

int main( int argc, char** argv ) {
    if( argc != 4 ) {
        std::cerr << "usage: proxy remote_host remote_port local_port\n\n"; return 1;
    }

    short remote_port = std::stoi( argv[2] );
    short local_port = std::stoi( argv[3] );

    proxy p( argv[1], remote_port, local_port );
    p.start();
}
