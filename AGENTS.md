## AGENTS.md - Enhanced Agent Workflow for ESP32 WiFi Serial Bridge

### Agent Architecture Overview

The project utilizes a modular agent architecture with specialized agents for different development aspects:

1. **Core Agent** - Primary coordination and firmware development expertise
2. **Build Agent** - PlatformIO compilation and build optimization
3. **Deployment Agent** - Firmware upload and OTA update management
4. **Documentation Agent** - Technical documentation and API generation
5. **Workflow Improver** - Continuous workflow refinement and optimization

### Agent Specifications

#### Core Agent (`core.md`)
- **Description**: Primary ESP32 firmware development agent coordinating build, deployment, and documentation workflows
- **Model**: `zai-coding-plan/glm-4.6v`
- **Temperature**: 0.1 (deterministic output)
- **Tools**: 
  - `write`, `edit`, `bash`, `webfetch`, `doom_loop`, `external_directory`
  - `platformio`, `make`
- **Focus Areas**:
  - Code quality and embedded C++ best practices
  - Bug detection and edge case analysis
  - Performance optimization for ESP32
  - Security considerations for embedded systems
  - Cross-functional workflow coordination
  - Build and deployment process management

#### Build Agent (`build/build.md`)
- **Description**: Specialized build agent for ESP32 firmware compilation and PlatformIO operations
- **Model**: `zai-coding-plan/glm-4.6v`
- **Temperature**: 0.1
- **Tools**: `write`, `edit`, `bash`, `platformio`, `make`
- **Focus Areas**:
  - PlatformIO build configuration optimization
  - C++ compilation flags and memory optimization
  - Build error analysis and resolution
  - Dependency management for embedded projects
  - Cross-compilation for ESP32 microcontrollers
  - Build time reduction strategies

#### Deployment Agent (`deployment/deployment.md`)
- **Description**: Specialized deployment agent for firmware uploads and OTA updates
- **Model**: `zai-coding-plan/glm-4.6v`
- **Temperature**: 0.1
- **Tools**: `write`, `edit`, `bash`, `platformio`, `make`
- **Focus Areas**:
  - Firmware upload processes (USB and OTA)
  - OTA update mechanisms and security protocols
  - Device programming and configuration management
  - Deployment workflow optimization
  - Error handling during firmware updates
  - Rollback strategies for failed deployments

#### Documentation Agent (`documentation/documentation.md`)
- **Description**: Specialized documentation agent for project documentation and API generation
- **Model**: `zai-coding-plan/glm-4.6v`
- **Temperature**: 0.1
- **Tools**: `write`, `edit`, `webfetch`
- **Focus Areas**:
  - API documentation generation and maintenance
  - README and deployment guide improvement
  - Code comments and documentation standards enforcement
  - User guides and configuration documentation
  - Documentation structure and organization
  - Technical writing for embedded systems

#### Workflow Improver (`agent-flow-improver.md`)
- **Description**: General agent to research and improve the agent workflow
- **Model**: `zai-coding-plan/glm-4.6v`
- **Temperature**: 0.1
- **Tools**: `write`, `edit`, `bash`, `webfetch`, `doom_loop`, `external_directory`
- **Focus Areas**:
  - Workflow efficiency analysis and optimization
  - Prompt refinement and improvement strategies
  - Agent coordination and communication enhancement
  - Tool usage optimization
  - Error handling and recovery mechanisms
  - Continuous improvement processes

### Workflow Orchestration

#### Primary Workflow
1. **Initial Assessment**: Core agent analyzes project requirements and current state
2. **Build Process**: Build agent handles compilation and optimization
3. **Deployment**: Deployment agent manages firmware upload and testing
4. **Documentation**: Documentation agent generates and updates technical docs
5. **Workflow Review**: Workflow improver agent evaluates and suggests improvements

#### Agent Communication Protocol
- **Status Updates**: Regular progress reports between agents
- **Error Handling**: Automated error detection and resolution workflows
- **Dependency Management**: Clear handoff between specialized agents
- **Review Cycles**: Peer review processes for quality assurance

### Development Workflow

#### Standard Development Cycle
1. **Feature Planning**: Core agent analyzes requirements and designs implementation
2. **Code Development**: Core agent implements features with build agent support
3. **Testing**: Automated testing (when available) and manual verification
4. **Documentation**: Documentation agent updates relevant docs
5. **Deployment**: Deployment agent handles firmware updates
6. **Review**: Workflow improver agent evaluates process efficiency

#### Build Process
```bash
# Primary build command
make build

# Full build with dependencies
make build-all

# Clean build (recommended for major changes)
make clean && make build
```

#### Deployment Process
```bash
# USB upload (development)
make upload

# OTA update (production)
make upload-ota IP=<device_ip>

# Full upload (firmware + filesystem)
make upload-all
```

### Testing Strategy

#### Current Testing Approach
- **PlatformIO Integration**: Leverages PlatformIO testing capabilities
- **Serial Monitoring**: Primary verification method (`make monitor`)
- **Functional Testing**: Web interface and MQTT communication testing
- **Unit Testing**: Limited unit tests in `test/` directory

#### Enhanced Testing Workflow
1. **Unit Testing**: Build agent runs unit tests before compilation
2. **Integration Testing**: Core agent coordinates integration tests
3. **System Testing**: Deployment agent handles system-level testing
4. **Documentation Testing**: Documentation agent verifies examples and guides

### Error Handling and Recovery

#### Common Error Scenarios
- **Build Failures**: Build agent provides detailed error analysis and resolution
- **Deployment Failures**: Deployment agent manages rollback procedures
- **Documentation Errors**: Documentation agent validates and corrects technical inaccuracies
- **Workflow Issues**: Workflow improver agent identifies and resolves process bottlenecks

#### Recovery Procedures
- **Automated Rollback**: Deployment agent maintains firmware version history
- **Build Cache Management**: Build agent optimizes cache for faster recovery
- **Documentation Versioning**: Documentation agent maintains documentation revisions

### Continuous Improvement

#### Monitoring and Metrics
- **Build Times**: Track compilation duration for optimization
- **Deployment Success Rates**: Monitor OTA update success rates
- **Documentation Quality**: Evaluate documentation completeness and accuracy
- **Workflow Efficiency**: Measure task completion times and agent coordination

#### Optimization Strategies
- **Parallel Processing**: Utilize parallel task execution where possible
- **Caching**: Implement intelligent caching for repeated operations
- **Prompt Refinement**: Continuously improve agent prompts based on performance
- **Tool Optimization**: Enhance tool usage patterns for better results

### Agent Coordination Best Practices

#### Communication Standards
- **Clear Handoffs**: Define clear responsibilities between agents
- **Status Reporting**: Regular progress updates and issue reporting
- **Decision Logging**: Document important decisions and rationale
- **Knowledge Sharing**: Maintain shared knowledge base across agents

#### Quality Assurance
- **Peer Review**: Implement cross-agent review processes
- **Validation Checks**: Include validation steps in workflows
- **Consistency Enforcement**: Maintain coding and documentation standards
- **Performance Monitoring**: Track and optimize agent performance

### Future Enhancements

#### Planned Improvements
- **Automated Testing Integration**: Expand testing capabilities
- **CI/CD Pipeline**: Implement continuous integration and deployment
- **Advanced Error Detection**: Improve error prediction and prevention
- **Performance Analytics**: Add detailed performance monitoring
- **Documentation Automation**: Enhance documentation generation capabilities

#### Research Areas
- **AI-assisted Debugging**: Explore AI-powered debugging tools
- **Predictive Maintenance**: Implement predictive failure detection
- **Self-Optimizing Workflows**: Develop adaptive workflow systems
- **Collaborative Development**: Enhance multi-agent collaboration

This enhanced workflow provides a comprehensive, modular approach to ESP32 firmware development with clear agent responsibilities, improved error handling, and continuous optimization capabilities.