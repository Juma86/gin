#include <header/aystl/sftp.hxx>

std::optional<std::unique_ptr<char[]>> aystl::net::sftp::ReadRemoteFile( aystl::net::host const& host
                                                                       , std::string const& remoteFilePath
    ) {

    auto ssh_free_session { [](ssh_session session) -> std::nullopt_t {
        ssh_disconnect(session);
        ssh_free(session);

        return std::nullopt;
    }   };
    
    auto error { [&ssh_free_session](ssh_session session) -> std::nullopt_t {
//        fprintf(stderr, "Error: %s\n", ssh_get_error(session));
        ssh_free_session(session);
        return std::nullopt;
    }   };

    ssh_session SSHSession;
    {
//        std::cout << "Generating new ssh session..." << std::endl;
        SSHSession = ssh_new();
        if (SSHSession == NULL) exit(-1);

      //  std::cout << host.name.c_str() << " | " << host.port << " | " << host.sftp->user.c_str() << std::endl;

        int port = std::stoi(host.port);
        ssh_options_set(SSHSession, SSH_OPTIONS_HOST, host.name.c_str());
        ssh_options_set(SSHSession, SSH_OPTIONS_PORT, &port);
        ssh_options_set(SSHSession, SSH_OPTIONS_USER, host.sftp->user.c_str());
    }

    {
 //       std::cout << "Connecting to host..." << std::endl;
        int rc = ssh_connect(SSHSession);
        if (rc != SSH_OK) return error(SSHSession);
    }

    {
     //   std::cout << "Password Authentication..." << std::endl;
        int rc = ssh_userauth_password(SSHSession, NULL, host.sftp->pass.c_str());
        if (rc != SSH_AUTH_SUCCESS) return error(SSHSession);
    }

    sftp_session SFTPSession;
    {
        SFTPSession = sftp_new(SSHSession);
        int rc = sftp_init(SFTPSession);
        if(rc != SSH_OK) { return error(SSHSession); }
    }

    sftp_file file;
    {
        file = sftp_open(SFTPSession, remoteFilePath.c_str(), O_RDONLY, S_IRWXU);
        if(file == NULL) { return error(SSHSession); }
    }

    ssize_t fileSize;
    {
        sftp_attributes fileAttributes = sftp_fstat(file);

        fileSize = fileAttributes->size;
    }
    

    std::unique_ptr<char[]> readBuffer ( new(std::nothrow) char[fileSize] );
    if(! readBuffer.get()) { sftp_close(file); sftp_free(SFTPSession); return ssh_free_session(SSHSession); }
    { // Read the subfile here
        constexpr auto BUFFER_SIZE = 2047;

        ssize_t read, read_total = 0;
        while ((read = sftp_read(file, readBuffer.get() +read_total, BUFFER_SIZE))) {
            read_total += read;
        }

        if(read_total != fileSize) { sftp_close(file); sftp_free(SFTPSession); return ssh_free_session(SSHSession); }
    }

    sftp_close(file);
    sftp_free(SFTPSession);
    ssh_free_session(SSHSession);

    return readBuffer;
}