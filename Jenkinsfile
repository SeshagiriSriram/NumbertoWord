pipeline {
    // Run globally on the built-in worker node
    agent any
    
    stages {
        stage('Checkout Source Code') {
            steps {
                // Securely pulls your Git repository over SSH
                checkout([$class: 'GitSCM', 
                    branches: [[name: '*/master']], 
                    extensions: [], 
                    userRemoteConfigs: [[
                        credentialsId: 'github-ssh-key', 
                        url: 'git@github.com:seshagirisriram/NumbertoWord'
                    ]]
                ])
            }
        }

        stage('Static C Code Analysis') {
            steps {
                echo '[CI-ANALYZER] Compiling the clean analysis toolchain environment...'
                // 1. Compile your custom analysis image file on the host engine cleanly
                sh 'docker build -t c-analyzer-suite:latest -f Dockerfile.Analysis .'
				
                echo '[CI-ANALYZER] Running static scans via shared named volume...'
                // 2. Run container using direct named-volume routing to bypass the DinD path visibility bug
                sh '''
                    docker run --rm \
                      -v jenkins-data:/var/jenkins_home \
                      -w /var/jenkins_home/workspace/${JOB_NAME} \
                      c-analyzer-suite:latest \
                      bash -c "
                        cppcheck --xml --xml-version=2 --enable=all --inconclusive main.c 2> cppcheck-result.xml && \
                        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON . || true && \
                        run-clang-tidy -p . > clang-tidy-result.log || true
                      "
                '''
            }
        }

        stage('Secure Container Build & Push') {
            steps {
                // Wraps execution inside the Jenkins log-masking environment vault block
                withCredentials([usernamePassword(credentialsId: 'docker-registry-creds', usernameVariable: 'REGISTRY_USER', passwordVariable: 'REGISTRY_PASS')]) {
                    
                    echo 'Authenticating to Docker Registry via GPG credential store...'
                    sh 'echo "$REGISTRY_PASS" | docker login --username "$REGISTRY_USER" --password-stdin'
                    
                    echo 'Building C application target deployment container...'
                    sh 'docker build -t seshagirisriram/c-app:latest .'
                    
                    echo 'Pushing secure image architecture layers...'
                    sh 'docker push seshagirisriram/c-app:latest'
                }
            }
        }
    }

    post {
        always {
            echo 'Processing logs into visual dashboard metrics...'
            // The unified Warnings plugin parses code smells and builds graphs natively on your project screen
            recordIssues(
                enabledForFailure: true,
                tools: [
                    cppCheck(pattern: 'cppcheck-result.xml'),
                    clangTidy(pattern: 'clang-tidy-result.log')
                ],
                qualityGates: [
                    [threshold: 1, type: 'TOTAL', severity: 'HIGH', unstable: false], 
                    [threshold: 10, type: 'TOTAL', severity: 'NORMAL', unstable: true] 
                ]
            )
        }
    }
}
