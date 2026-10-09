pipeline {
    // Tells the pipeline that unless specified otherwise by a stage, run on any available worker node
    agent any
    stages {
        stage('Checkout Source Code') {
            steps {
                // Securely pulls your Git repository over SSH using your pre-seeded host key policy
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
            agent {
                dockerfile {
                    // Boots your custom toolchain image instantly from the workspace root
                    filename 'Dockerfile.analysis'
                    args '-u root' 
                }
            }
            steps {
                // CLEAN UP: Removed the apt-get install step since tools are already pre-baked!
                echo '[CI-ANALYZER] Executing Cppcheck static analysis profile...'
                sh 'cppcheck --xml --xml-version=2 --enable=all --inconclusive . 2> cppcheck-result.xml'
                
                echo '[CI-ANALYZER] Generating compilation maps for Clang-Tidy...'
                sh 'cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON . || true'
                
                echo '[CI-ANALYZER] Executing Clang-Tidy code reviews...'
                sh 'run-clang-tidy -p . > clang-tidy-result.log || true'
            }
        }

        stage('Secure Container Build & Push') {
            steps {
                // Wraps execution inside the Jenkins log-masking environment vault block
                withCredentials([usernamePassword(credentialsId: 'docker-registry-creds', usernameVariable: 'REGISTRY_USER', passwordVariable: 'REGISTRY_PASS')]) {
                    
                    echo 'Authenticating to Docker Registry via GPG credential store...'
                    // Jenkins will dynamically catch and substitute this with **** in console outputs
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
                // Quality gate rules: automatically fail or destabilize the build based on bug severity
                qualityGates: [
                    [threshold: 1, type: 'TOTAL', severity: 'HIGH', unstable: false], // Fail if severe bugs exist
                    [threshold: 10, type: 'TOTAL', severity: 'NORMAL', unstable: true] // Unstable if styling warnings > 10
                ]
            )
        }
    }
}
