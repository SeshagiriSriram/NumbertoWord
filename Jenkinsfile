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
                script {
                    echo '[CI-ANALYZER] Compiling the clean analysis toolchain environment...'
                    // 1. Force a clean local compile of your Analysis image file
                    def analyzerImage = docker.build("c-analyzer-suite:${env.BUILD_NUMBER}", "-f Dockerfile.Analysis .")
                    
                    echo '[CI-ANALYZER] Booting toolchain container...'
                    // 2. Run inside the container block natively (Bypasses the agent proxy bug completely)
                    analyzerImage.inside('-u root') {
                        echo '[CI-ANALYZER] Running Cppcheck static scan...'
                        sh 'cppcheck --xml --xml-version=2 --enable=all --inconclusive main.c 2> cppcheck-result.xml'
                        
                        echo '[CI-ANALYZER] Generating compilation maps for Clang-Tidy...'
                        sh 'cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON . || true'
                        
                        echo '[CI-ANALYZER] Executing Clang-Tidy code reviews...'
                        sh 'run-clang-tidy -p . > clang-tidy-result.log || true'
                    }
                }
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
