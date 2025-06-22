document.addEventListener('DOMContentLoaded', function() {
    // 配置marked.js
    marked.setOptions({
        highlight: function(code, lang) {
            const language = hljs.getLanguage(lang) ? lang : 'plaintext';
            return hljs.highlight(code, { language }).value;
        },
        breaks: true,
        gfm: true
    });

    const chatHistory = document.getElementById('chat-history');
    const userInput = document.getElementById('user-input');
    const sendBtn = document.getElementById('send-btn');
    const clearBtn = document.getElementById('clear-btn');
    
    const WORKER_URL = 'dut-zenith.top';
    let conversationHistory = [];
    
    async function sendMessage() {
        const message = userInput.value.trim();
        if (!message) return;
        
        sendBtn.disabled = true;
        userInput.disabled = true;
        
        addMessageToChat('user', message);
        userInput.value = '';
        
        const thinkingMsgId = showThinkingMessage();
        
        try {
            const response = await callDeepSeekAPI(message);
            removeThinkingMessage(thinkingMsgId);
            
            if (response && response.choices && response.choices[0].message.content) {
                const aiResponse = response.choices[0].message.content;
                addMessageToChat('assistant', aiResponse, true); // 标记为需要渲染Markdown
                
                conversationHistory.push(
                    { role: 'user', content: message },
                    { role: 'assistant', content: aiResponse }
                );
            } else {
                throw new Error('无效的API响应');
            }
        } catch (error) {
            removeThinkingMessage(thinkingMsgId);
            console.error('API调用失败:', error);
            addMessageToChat('assistant', `抱歉，出现错误: ${error.message}`);
        } finally {
            sendBtn.disabled = false;
            userInput.disabled = false;
            userInput.focus();
        }
    }
    
    async function callDeepSeekAPI(message) {
        try {
            const response = await fetch(WORKER_URL, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    model: 'deepseek-chat',
                    messages: [
                        ...conversationHistory,
                        { role: 'user', content: message }
                    ]
                })
            });
            
            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.error || `请求失败: ${response.status}`);
            }
            
            return response.json();
        } catch (error) {
            console.error('Fetch错误:', error);
            throw new Error(`无法连接到AI服务: ${error.message}`);
        }
    }
    
    // 修改后的消息添加函数，支持Markdown渲染，AI消息统一样式
    function addMessageToChat(role, content, isMarkdown = false) {
        const messageDiv = document.createElement('div');
        if (role === 'assistant') {
            // 直接用ai-message作为内容div的class，防止被marked渲染顶层div覆盖
            messageDiv.className = 'message ai-message';
            messageDiv.style.display = 'flex';
            messageDiv.style.alignItems = 'flex-start';

            const avatar = document.createElement('img');
            avatar.src = 'https://smallgoodgood.top/images/23.jpg';
            avatar.className = 'cat-avatar';
            messageDiv.appendChild(avatar);

            const contentDiv = document.createElement('div');
            contentDiv.style.flex = '1';
            if (isMarkdown) {
                contentDiv.innerHTML = marked.parse(content);
            } else {
                contentDiv.textContent = content;
            }
            messageDiv.appendChild(contentDiv);
        } else {
            messageDiv.className = 'message user-message';
            const messageContent = document.createElement('div');
            messageContent.textContent = content;
            messageDiv.appendChild(messageContent);
        }

        const messageTime = document.createElement('div');
        messageTime.className = 'message-time';
        messageTime.textContent = new Date().toLocaleTimeString();
        messageDiv.appendChild(messageTime);

        chatHistory.appendChild(messageDiv);
        chatHistory.scrollTop = chatHistory.scrollHeight;

        // 高亮代码块
        if (isMarkdown && role === 'assistant') {
            messageDiv.querySelectorAll('pre code').forEach((block) => {
                hljs.highlightElement(block);
            });
        }
    }

    // AI思考消息也用相同结构和样式
    function showThinkingMessage() {
        const messageDiv = document.createElement('div');
        messageDiv.className = 'message ai-message';
        messageDiv.id = 'thinking-message';

        const innerDiv = document.createElement('div');
        innerDiv.style.display = 'flex';
        innerDiv.style.alignItems = 'flex-start';

        const avatar = document.createElement('img');
        avatar.src = 'https://smallgoodgood.top/images/23.jpg';
        avatar.className = 'cat-avatar';
        innerDiv.appendChild(avatar);

        const contentDiv = document.createElement('div');
        const thinkingText = document.createElement('span');
        thinkingText.textContent = '正在思考...';
        contentDiv.appendChild(thinkingText);
        const loadingSpan = document.createElement('span');
        loadingSpan.className = 'loading';
        contentDiv.appendChild(loadingSpan);
        innerDiv.appendChild(contentDiv);

        messageDiv.appendChild(innerDiv);

        const messageTime = document.createElement('div');
        messageTime.className = 'message-time';
        messageTime.textContent = new Date().toLocaleTimeString();
        messageDiv.appendChild(messageTime);

        chatHistory.appendChild(messageDiv);
        chatHistory.scrollTop = chatHistory.scrollHeight;

        return 'thinking-message';
    }
    
    function removeThinkingMessage(messageId) {
        const thinkingMsg = document.getElementById(messageId);
        if (thinkingMsg) {
            thinkingMsg.remove();
        }
    }
    
    function clearChatHistory() {
        chatHistory.innerHTML = '';
        conversationHistory = [];
    }
    
    sendBtn.addEventListener('click', sendMessage);
    
    userInput.addEventListener('keydown', function(e) {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });
    
    clearBtn.addEventListener('click', clearChatHistory);
    
    // 限制输入字数为500
    userInput.setAttribute('maxlength', '500');

    // 字数统计提示移到左下角
    let charCountTip = document.createElement('div');
    charCountTip.style.position = 'absolute';
    charCountTip.style.left = '16px';
    charCountTip.style.bottom = '12px';
    charCountTip.style.fontSize = '12px';
    charCountTip.style.color = '#888';
    charCountTip.style.zIndex = '2';
    userInput.parentNode.appendChild(charCountTip);

    function updateCharCount() {
        const len = userInput.value.length;
        charCountTip.textContent = `${len}/500`;
        if (len >= 500) {
            charCountTip.style.color = '#e74c3c';
        } else {
            charCountTip.style.color = '#888';
        }
    }
    userInput.addEventListener('input', updateCharCount);
    updateCharCount();

    userInput.focus();
});