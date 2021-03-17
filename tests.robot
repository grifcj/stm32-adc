*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Setup                    My Test Setup
Test Teardown                 Test Teardown
Resource                      ${RENODEKEYWORDS}

*** Keywords ***
My Test Setup
    Reset Emulation
    Execute Command         path add @${CURDIR}

Feed Constant
    [Arguments]       ${sample}  ${channel}
    Execute Command   sysbus.adc1 FeedSample ${sample} ${channel} -1

# Feed SawTooth
#     [Arguments]       ${sample}
#     Execute Command   sysbus.adc1 FeedSample ${sample}
#
# Feed SineWave
#     [Arguments]       ${sample}
#     Execute Command   sysbus.adc1 InputConstant ${sample}

Received Constant
    [Arguments]             ${sample}
    Wait For Line On Uart   Data read: ${sample}  timeout=0.2
    Wait For Line On Uart   Data read: ${sample}  timeout=0.2
    Wait For Line On Uart   Data read: ${sample}  timeout=0.2

# Received SawTooth
#     [Arguments]             ${start}
#     Wait For Line On Uart   Data read: ${start}  timeout=0.2
#     Wait For Line On Uart   Data read: ${start} + 1  timeout=0.2
#     Wait For Line On Uart   Data read: ${start} + 2  timeout=0.2

*** Test Cases ***
Polling ADC can read sample
    Execute Command         include @renode-polling.resc

    Create Terminal Tester  sysbus.uart1

    Start Emulation

    Feed Constant  sample=1  channel=1
    Received Constant  sample=1

    Feed Constant  sample=5  channel=1
    Received Constant  sample=5

Interrupt ADC can read sample
    Execute Command         include @renode-interrupt.resc

    Create Terminal Tester  sysbus.uart1

    Start Emulation

    Feed Constant  sample=1  channel=1
    Received Constant  sample=1

    Feed Constant  sample=5  channel=1
    Received Constant  sample=5

