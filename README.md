# pokebell

Project to reproduce pagers in modern times using AWS, SORACOM, M5Stack.

```bash
.
├── Makefile                    <-- Make to automate build
├── README.md                   <-- This instructions file
├── arduino                     <-- Source code for arduino(M5Stack)
│   ├── receiver                <-- Source code for receiver
│   │   └── pokebell.ino        <-- Receiver code
│   ├── sender                  <-- Source code for sender
│   │   └── pokebell.ino        <-- Sender code
├── funk                        <-- Source code for a lambda function
│   ├── app.rb                  <-- Lambda function code
│   └── Gemfile                 <-- Gemfile
└── template.yaml
```

## Requirements

* AWS CLI already configured with Administrator permission
* [Docker installed](https://www.docker.com/community-edition)
* [Ruby](https://www.ruby-lang.org/)
* SAM CLI - [Install the SAM CLI](https://docs.aws.amazon.com/serverless-application-model/latest/developerguide/serverless-sam-cli-install.html)
