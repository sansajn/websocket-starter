pipeline {
	agent any
	stages {
		stage('clean') { steps {
			sh 'scons -c'

			// remove .gcda fragments
			sh 'find  . -depth -type f -name \'*.gcda\' -exec rm \'{}\' \';\''
		}} 

		stage('build') { steps {
			sh 'scons -j8 eserv'
			sh 'scons -j8 --test-coverage test'
		}}

		stage('test') { steps {
			sh './test -r junit -o junit.report' 
		}}

		stage('static analyze') { steps {
			sh 'cppcheck -j8 --xml --xml-version=2 --output-file=cppcheck.report .'
		}}

		stage('test coverage analyze') { steps {
			sh 'gcovr -r . --xml-pretty -o coverage.report'
		}}
	}
	post { always {
		// report test results
		junit 'junit.report'

		// report GCC warnings
		recordIssues tool: gcc()

		// report static analyze
		recordIssues tool: cppCheck(pattern: 'cppcheck.report')

		// report test coverage
		cobertura coberturaReportFile: 'coverage.report'
	}}
}
